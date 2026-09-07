/*
 * rwatracemain.cpp — headless trace harness for the RWA engine.
 *
 * Loads a .rwa game, replays a scripted scenario (GPS positions, head
 * orientation, steps, playfinished events) against RwaRuntime at the
 * Creator's native 25 ms tick, and writes a JSONL trace of every pd message
 * the engine emits plus every scene/state transition.
 *
 *   rwatrace --game <game.rwa> --scenario <scenario.json> --out <trace.jsonl>
 *
 * The same scenario files are meant to be replayed by the iOS Player's test
 * target (RwaGameLoop) so the two traces can be diffed — see
 * docs/engine-runtime-investigation.md.
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

#include <map>
#include <mutex>
#include <string>

#include "rwaruntime.h"
#include "rwaimport.h"
#include "rwaentity.h"
#include "fake_libpd.h"

namespace {

QFile traceFile;
QTextStream traceOut;

int currentTick = 0;
int currentTimeMs = 0;

// patcherTag → asset basename, learned from the "<tag>-play" symbol message
std::map<int, QString> tagToAsset;
// asset basename → most recent patcherTag playing it (for playfinished injection)
std::map<QString, int> lastTagForAsset;

void writeEvent(QJsonObject obj)
{
    obj.insert("t", currentTimeMs);
    obj.insert("tick", currentTick);
    traceOut << QJsonDocument(obj).toJson(QJsonDocument::Compact) << "\n";
}

int tagOfReceiver(const QString &receiver)
{
    int dash = receiver.indexOf('-');
    if(dash <= 0)
        return -1;
    bool ok = false;
    int tag = receiver.left(dash).toInt(&ok);
    return ok ? tag : -1;
}

void recordPdMessage(char kind, const char *recv, float val, const char *sym)
{
    QString receiver = QString::fromUtf8(recv);

    QJsonObject obj;
    obj.insert("ev", "pd");
    obj.insert("recv", receiver);

    if(kind == 'f')
        obj.insert("val", static_cast<double>(val));
    else if(kind == 'b')
        obj.insert("bang", true);
    else if(kind == 's')
        obj.insert("sym", QString::fromUtf8(sym));

    int tag = tagOfReceiver(receiver);
    if(tag > 0)
    {
        if(kind == 's' && receiver.endsWith("-play"))
        {
            QString asset = QFileInfo(QString::fromUtf8(sym)).fileName();
            tagToAsset[tag] = asset;
            lastTagForAsset[asset] = tag;
        }
        auto it = tagToAsset.find(tag);
        if(it != tagToAsset.end())
            obj.insert("asset", it->second);
    }

    writeEvent(obj);
}

QString locationName(RwaLocation1 *location)
{
    return location ? QString::fromStdString(location->objectName()) : QString();
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("rwatrace");

    QCommandLineParser parser;
    parser.setApplicationDescription("Headless RWA engine trace harness");
    parser.addHelpOption();
    QCommandLineOption gameOption("game", "Path to the .rwa game file.", "file");
    QCommandLineOption scenarioOption("scenario", "Path to the scenario JSON.", "file");
    QCommandLineOption outOption("out", "Output trace path (JSONL).", "file", "trace.jsonl");
    parser.addOption(gameOption);
    parser.addOption(scenarioOption);
    parser.addOption(outOption);
    parser.process(app);

    const QString gamePath = parser.value(gameOption);
    const QString scenarioPath = parser.value(scenarioOption);
    const QString outPath = parser.value(outOption);

    if(gamePath.isEmpty() || scenarioPath.isEmpty())
    {
        QTextStream(stderr) << "error: --game and --scenario are required\n";
        return 2;
    }

    /* ------------------------------- scenario ------------------------------ */

    QFile scenarioFile(scenarioPath);
    if(!scenarioFile.open(QIODevice::ReadOnly))
    {
        QTextStream(stderr) << "error: cannot open scenario " << scenarioPath << "\n";
        return 2;
    }
    QJsonParseError parseError;
    const QJsonObject scenario = QJsonDocument::fromJson(scenarioFile.readAll(), &parseError).object();
    if(parseError.error != QJsonParseError::NoError)
    {
        QTextStream(stderr) << "error: scenario JSON: " << parseError.errorString() << "\n";
        return 2;
    }
    const QJsonArray timeline = scenario.value("timeline").toArray();
    const int durationMs = scenario.value("durationMs").toInt(60000);
    const QString startSceneName = scenario.value("startScene").toString();

    /* ------------------------------ game import ---------------------------- */

    const QString gameDir = QFileInfo(gamePath).absolutePath();

    QList<RwaScene *> scenes;
    RwaImport import(nullptr, &scenes, gameDir);

    QFile gameFile(gamePath);
    if(!gameFile.open(QIODevice::ReadOnly))
    {
        QTextStream(stderr) << "error: cannot open game " << gamePath << "\n";
        return 2;
    }
    if(!import.read(&gameFile) || scenes.isEmpty())
    {
        QTextStream(stderr) << "error: failed to import " << gamePath << "\n";
        return 2;
    }

    /* ------------------------------ trace output --------------------------- */

    traceFile.setFileName(outPath);
    if(!traceFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        QTextStream(stderr) << "error: cannot open output " << outPath << "\n";
        return 2;
    }
    traceOut.setDevice(&traceFile);

    /* --------------------- engine setup (mirrors RwaSimulator) ------------- */

    RwaEntity *entity = new RwaEntity("Gandalf", RWAENTITYTYPE_HERO);
    entity->scenes = std::list<RwaScene *>(scenes.begin(), scenes.end());

    const std::string assetPath = (gameDir + "/assets/").toStdString();
    const float schedulerRate = 25;
    std::mutex pdMutex;

    RwaRuntime *runtime = new RwaRuntime(nullptr, gameDir.toStdString().c_str(),
                                         assetPath.c_str(), 48000, schedulerRate,
                                         &pdMutex, nullptr);
    runtime->entities = std::list<RwaEntity *>{entity};

    // deterministic trace: the "<tag>-seed" init value is a platform-RNG draw in
    // production, pin it here (the engine sends 1 + (source & 0xFFFFFE), i.e. 1)
    RwaRuntime::seedSource = []{ return 0u; };

    RwaScene *startScene = scenes.front();
    for(RwaScene *scene : scenes)
    {
        if(locationName(scene) == startSceneName)
            startScene = scene;
    }

    fake_libpd_set_recorder(recordPdMessage);

    runtime->unblockStates(entity);
    runtime->initDynamicPdPatchers(entity);
    runtime->setScene(entity, startScene);

    QString lastScene = locationName(entity->getCurrentScene());
    QString lastState = locationName(entity->getCurrentState());
    {
        QJsonObject obj;
        obj.insert("ev", "state");
        obj.insert("scene", lastScene);
        obj.insert("state", lastState);
        writeEvent(obj);
    }

    /* -------------------------------- tick loop ----------------------------- */

    int timelineIndex = 0;
    const int totalTicks = durationMs / static_cast<int>(schedulerRate);

    for(currentTick = 1; currentTick <= totalTicks; currentTick++)
    {
        currentTimeMs = currentTick * static_cast<int>(schedulerRate);

        while(timelineIndex < timeline.size()
              && timeline.at(timelineIndex).toObject().value("t").toInt() <= currentTimeMs)
        {
            const QJsonObject input = timeline.at(timelineIndex).toObject();
            timelineIndex++;

            QJsonObject echo;
            echo.insert("ev", "input");

            if(input.contains("pos"))
            {
                const QJsonObject pos = input.value("pos").toObject();
                std::vector<double> coordinates(2, 0.0);
                coordinates[0] = pos.value("lon").toDouble();
                coordinates[1] = pos.value("lat").toDouble();
                entity->setCoordinates(coordinates);
                echo.insert("kind", "pos");
                echo.insert("lon", coordinates[0]);
                echo.insert("lat", coordinates[1]);
                writeEvent(echo);
            }
            if(input.contains("azimuth"))
            {
                entity->setAzimuth(input.value("azimuth").toDouble());
                QJsonObject echoAz{{"ev", "input"}, {"kind", "azimuth"}, {"value", input.value("azimuth")}};
                writeEvent(echoAz);
            }
            if(input.contains("elevation"))
            {
                entity->setElevation(input.value("elevation").toDouble());
                QJsonObject echoEl{{"ev", "input"}, {"kind", "elevation"}, {"value", input.value("elevation")}};
                writeEvent(echoEl);
            }
            if(input.value("step").toBool())
            {
                runtime->step = true;
                QJsonObject echoStep{{"ev", "input"}, {"kind", "step"}};
                writeEvent(echoStep);
            }
            if(input.contains("playFinished"))
            {
                const QJsonObject pf = input.value("playFinished").toObject();
                if(pf.contains("tag"))
                {
                    // explicit tag, for addressing an older instance of an asset
                    // that has been restarted since (the name lookup below always
                    // resolves to the newest instance)
                    const int tag = pf.value("tag").toInt();
                    fake_libpd_inject_bang(QString("%1-playfinished").arg(tag).toStdString());
                    QJsonObject echoPf{{"ev", "input"}, {"kind", "playFinished"}, {"tag", tag}};
                    writeEvent(echoPf);
                }
                else
                {
                    const QString asset = pf.value("asset").toString();
                    auto it = lastTagForAsset.find(asset);
                    if(it != lastTagForAsset.end())
                    {
                        fake_libpd_inject_bang(QString("%1-playfinished").arg(it->second).toStdString());
                        QJsonObject echoPf{{"ev", "input"}, {"kind", "playFinished"}, {"asset", asset}, {"tag", it->second}};
                        writeEvent(echoPf);
                    }
                    else
                    {
                        QJsonObject echoPf{{"ev", "input"}, {"kind", "playFinished"}, {"asset", asset}, {"error", "asset not playing"}};
                        writeEvent(echoPf);
                    }
                }
            }
        }

        runtime->update(entity);

        const QString sceneName = locationName(entity->getCurrentScene());
        const QString stateName = locationName(entity->getCurrentState());
        if(sceneName != lastScene || stateName != lastState)
        {
            QJsonObject obj;
            obj.insert("ev", "state");
            obj.insert("scene", sceneName);
            obj.insert("state", stateName);
            obj.insert("prevScene", lastScene);
            obj.insert("prevState", lastState);
            writeEvent(obj);
            lastScene = sceneName;
            lastState = stateName;
        }
    }

    traceOut.flush();
    traceFile.close();
    QTextStream(stdout) << "trace written to " << outPath << "\n";
    return 0;
}
