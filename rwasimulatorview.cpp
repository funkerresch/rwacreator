#include "rwasimulatorview.h"

void RwaSimulatorView::selectOutputDevice(int index)
{
    (void) index;
    startStopAudio(false);
    simulator->ap->setOutputDevice(selectOutputCombo->currentData().toInt());
}

void RwaSimulatorView::selectInputDevice(int index)
{
    (void) index;
    startStopAudio(false);
    simulator->ap->setInputDevice(selectInputCombo->currentData().toInt());
}

void RwaSimulatorView::createPortaudioControls(const QString &title)
{
    portaudioControlsGroup = new QGroupBox(title);
    selectOutputCombo = new QComboBox;
    selectOutputCombo->setFixedHeight(24);
    selectInputCombo = new QComboBox;
    selectInputCombo->setFixedHeight(24);
    startStopButton = new QPushButton;
    startStopButton->setText(tr("Start Audio"));
    startStopButton->setCheckable(true);

    connect(startStopButton, SIGNAL(toggled(bool)), this, SLOT(startStopAudio(bool)));

    const PaDeviceInfo* di;

    for(int i = 0;i < Pa_GetDeviceCount();i++)
    {
        if(simulator->ap->isOutputDevice(i))
        {
             di = Pa_GetDeviceInfo(i);
             selectOutputCombo->addItem(QString(di->name), i);
        }
    }

    for(int i = 0;i < Pa_GetDeviceCount();i++)
    {
        if(simulator->ap->isInputDevice(i))
        {
             di = Pa_GetDeviceInfo(i);
             selectInputCombo->addItem(QString(di->name), i);
        }
    }

    QGridLayout *controlsLayout = new QGridLayout;
    controlsLayout->addWidget(selectOutputCombo, 0, 0, 1, 3);
    controlsLayout->addWidget(selectInputCombo, 1, 0, 1, 3);
    controlsLayout->addWidget(startStopButton, 2, 0, 1, 3);
    portaudioControlsGroup->setLayout(controlsLayout);

    connect(selectOutputCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(selectOutputDevice(int)));
    connect(selectInputCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(selectInputDevice(int)));
}

void RwaSimulatorView::startStopAudio(bool startStop)
{
   if(startStop)
   {
       startStopButton->setText(tr("Stop Simulation"));
       startStopButton->setChecked(true);
       simulator->startRwaSimulation();
   }
   else
   {
       startStopButton->setText(tr("Start Simulation"));
       startStopButton->setChecked(false);
       simulator->stopRwaSimulation();
   }
}

RwaSimulatorView::RwaSimulatorView(QWidget* parent, RwaScene *scene)
: RwaView(parent, scene)
{
    setAcceptDrops(true);
    layout = new QBoxLayout(QBoxLayout::TopToBottom,this);

    currentScene = NULL;
    currentState = NULL;
    simulator = backend->simulator;

    createPortaudioControls("Audio Control");
    layout->addWidget(portaudioControlsGroup);
}

void RwaSimulatorView::resetAssets()
{
    sendResetAssets();
}

void RwaSimulatorView::receiveSelectionChanged()
{
    ;//
}

void RwaSimulatorView::adaptSize(qint32 width, qint32 height)
{
    resize(QSize(width, height));
  //  mc->resize(QSize(width/2, height-40));

}

void RwaSimulatorView::dropEvent(QDropEvent *event)
{
    qDebug("drop");

}

void RwaSimulatorView::setCurrentState(RwaState *currentState)
{

}


