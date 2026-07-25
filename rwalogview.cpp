#include "rwalogview.h"
#include <QMetaType>
#include <QVBoxLayout>
#include <QtLogging>
#include "rwabackend.h"

RwaLogWindow::RwaLogWindow(QWidget *parent) :
 QWidget(parent)
{
    backend = RwaBackend::getInstance();
    qRegisterMetaType<QtMsgType>("QtMsgType");
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    setLayout(layout);
    logView = new QPlainTextEdit(this);
    logView->setReadOnly(true);
    logView->setStyleSheet("QPlainTextEdit { font-family: 'Andale Mono', monospace; font-size: 12pt; }");
    layout->addWidget(logView);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(4, 4, 4, 4);
    layout->addLayout(buttonLayout);

    // Left: clear button
    clearButton = new QPushButton(this);
    clearButton->setText("clear");
    buttonLayout->addWidget(clearButton);
    connect(clearButton, SIGNAL (clicked()), logView, SLOT (clear()));

    buttonLayout->addStretch(1);

    // Middle: filter toggles
    logLongAndLatCheckbox = new QCheckBox(this);
    logLongAndLatCheckbox->setText("Coords");
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

    for (QCheckBox *cb : {logLongAndLatCheckbox, logLibPdPrint, logSimulatorStates, logOther})
        cb->setMinimumWidth(cb->sizeHint().width() + 10);

    buttonLayout->addStretch(1);

    // Right: log level selector
    buttonLayout->addWidget(new QLabel("Log Level:", this));
    logLevelComboBox = new QComboBox(this);
    logLevelComboBox->addItems({"Debug", "Info", "Warning", "Critical", "Fatal"});
    logLevelComboBox->setCurrentIndex(1); // Info
    buttonLayout->addWidget(logLevelComboBox);

    connect(logLevelComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
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

    // extract filename safely
    // quirky fix around qSetMessagePattern showing the whole path
    QString file = (type == QtDebugMsg && context.file) ? QString::fromUtf8(context.file) : QString();
    if (!file.isEmpty())
        file = " (" + QFileInfo(file).fileName();

    qSetMessagePattern("[%{time hh:mm:ss.zzz}] [%{type}]\t%{message}%{if-debug}:%{line}, %{function})%{endif}");
    logView->appendPlainText(qFormatLogMessage(type, context, msg + file));
}
