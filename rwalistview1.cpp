#include "rwalistview1.h"

RwaListView1::RwaListView1(QWidget *parent)
{
    setParent(parent);
    backend = RwaBackend::getInstance();

    connect (backend, SIGNAL(newGameLoaded()),
             this, SLOT(receiveNewGameSignal()));

    connect(backend, SIGNAL(sendLastTouchedState(RwaState *)),
              this, SLOT(setCurrentState(RwaState*)));

//    connect(backend, SIGNAL(sendLastTouchedScene(RwaScene *)),
//              this, SLOT(setCurrentScene(RwaScene*)));

//    connect(backend, SIGNAL(sendLastTouchedAsset(RwaAsset1 *)),
//              this, SLOT(setCurrentAsset(RwaAsset1*)));

    listModel = new QStringListModel(this);
    this->setModel(listModel);

}

