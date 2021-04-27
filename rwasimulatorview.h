#ifndef RWASIMULATORVIEW_H
#define RWASIMULATORVIEW_H

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <rwaview.h>
#include "rwaassetlist.h"
#include "rwasimulator.h"

class RwaSimulatorView : public RwaView
{
    Q_OBJECT
public:
    explicit RwaSimulatorView(QWidget* parent = 0, RwaScene *scene = 0);
    void createPortaudioControls(const QString &title);
    QComboBox *selectOutputCombo;
    QComboBox *selectInputCombo;
    QGroupBox *portaudioControlsGroup;
    QPushButton *startStopButton;
private:

    RwaSimulator *simulator;



public slots:
    void resetAssets();

    void dropEvent(QDropEvent *event);

    void adaptSize(qint32 width, qint32 height);
    void setCurrentState(RwaState *currentState);
    void receiveSelectionChanged();
    void startStopAudio(bool startStop);
    void selectOutputDevice(int index);
    void selectInputDevice(int index);

protected:

signals:
     void sendResetAssets();
};

#endif // RWASIMULATORVIEW_H
