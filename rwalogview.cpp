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
     browser = new QTextBrowser(this);
     layout->addWidget(browser);

      QHBoxLayout *buttonLayout = new QHBoxLayout;
      buttonLayout->setContentsMargins(0, 0, 0, 0);
      layout->addLayout(buttonLayout);

      buttonLayout->addStretch(10);

      clearButton = new QPushButton(this);
      clearButton->setText("clear");
      buttonLayout->addWidget(clearButton);
      connect(clearButton, SIGNAL (clicked()), browser, SLOT (clear()));

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
}

RwaLogWindow::~RwaLogWindow()
{

}

void RwaLogWindow::outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    if(localMsg.size() > 512)
        return;

    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    char output[1024];

    switch (type)
    {
        case QtDebugMsg:
        sprintf(output, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        browser->append(output);
        break;

        case QtInfoMsg:
        break;

        case QtWarningMsg:
        //browser->append(tr("— WARNING: %1").arg(msg));
        break;

        case QtCriticalMsg:
        //browser->append(tr("— CRITICAL: %1").arg(msg));
        break;

        case QtFatalMsg:
        //browser->append(tr("— FATAL: %1").arg(msg));
        break;
    }
}
