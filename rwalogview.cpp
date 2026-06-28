#include "rwalogview.h"
#include <QMetaType>
#include <QVBoxLayout>
#include "rwabackend.h"

RwaLogWindow::RwaLogWindow(QWidget *parent) :
 QWidget(parent)
{
    backend = RwaBackend::getInstance();
    qRegisterMetaType<QtMsgType>("QtMsgType");
    QVBoxLayout *layout = new QVBoxLayout;

    setLayout(layout);
    logView = new QPlainTextEdit(this);
    logView->setReadOnly(true);
    layout->addWidget(logView);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(buttonLayout);

    buttonLayout->addStretch(10);

    clearButton = new QPushButton(this);
    clearButton->setText("clear");
    buttonLayout->addWidget(clearButton);
    connect(clearButton, SIGNAL (clicked()), logView, SLOT (clear()));

    logLongAndLatCheckbox = new QCheckBox(this);
    logLongAndLatCheckbox->setText("Lon & Lat");
    buttonLayout->addWidget(logLongAndLatCheckbox);
    connect(logLongAndLatCheckbox, SIGNAL (stateChanged(int)), backend, SLOT (receiveLogLonAndLat(int)) );

    logLibPdPrint = new QCheckBox(this);
    logLibPdPrint->setText("LibPd");
    buttonLayout->addWidget(logLibPdPrint);
    connect(logLibPdPrint, SIGNAL (stateChanged(int)), backend, SLOT (receiveLogLibPd(int)) );

    logSimulatorStates = new QCheckBox(this);
    logSimulatorStates->setText("Simulator");
    buttonLayout->addWidget(logSimulatorStates);
    connect(logSimulatorStates, SIGNAL (stateChanged(int)), backend, SLOT (receiveLogSimulator(int)) );

    logOther = new QCheckBox(this);
    logOther->setText("Other");
    buttonLayout->addWidget(logOther);
    connect(logOther, SIGNAL (stateChanged(int)), backend, SLOT (receiveLogOther(int)) );
    buttonLayout->addWidget(new QLabel("Log Level:", this));
    logLevelComboBox = new QComboBox(this);
    logLevelComboBox->addItems({"Debug", "Info", "Warning", "Critical", "Fatal"});
    logLevelComboBox->setCurrentIndex(1); // Info
    buttonLayout->addWidget(logLevelComboBox);

    connect(logLevelComboBox, &QComboBox::currentIndexChanged, this, [this](int index) {
        static const QtMsgType levels[] = { QtDebugMsg, QtInfoMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg };
        logLevel = levels[index];
    });
}

RwaLogWindow::~RwaLogWindow()
{

}

static int msgSeverity(QtMsgType type)
{
    switch(type) {
        case QtDebugMsg:    return 0;
        case QtInfoMsg:     return 1;
        case QtWarningMsg:  return 2;
        case QtCriticalMsg: return 3;
        case QtFatalMsg:    return 4;
    }
    return 0;
}

void RwaLogWindow::outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (msgSeverity(type) < msgSeverity(logLevel))
        return;

    if (msg.length() > 512) {
        return;
    }

    // extract context safely
    QString file = context.file ? QString::fromUtf8(context.file) : QString();
    QString function = context.function ? QString::fromUtf8(context.function) : QString();

    if (!file.isEmpty()) {
        file = QFileInfo(file).fileName();
    }

    QString prefix;
    switch (type) {
        case QtDebugMsg:    prefix = QStringLiteral("Debug"); break;
        case QtInfoMsg:     prefix = QStringLiteral("Info"); break;
        case QtWarningMsg:  prefix = QStringLiteral("Warning"); break;
        case QtCriticalMsg: prefix = QStringLiteral("Critical"); break;
        case QtFatalMsg:    prefix = QStringLiteral("Fatal"); break;
    }

    QString output = QStringLiteral("%1: %2 (%3:%4, %5)")
                        .arg(prefix, msg, file)
                        .arg(context.line)
                        .arg(function);

    logView->appendPlainText(output);
}
