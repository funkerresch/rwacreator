#include "rwaassetlist1.h"

RwaAssetList1::RwaAssetList1(QWidget *parent)
    :RwaListView1(parent)
{
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DropOnly);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void RwaAssetList1::update()
{
    qDebug();

}

void RwaAssetList1::receiveNewGameSignal()
{
    listModel->setStringList(backend->assetStringList);
}

void RwaAssetList1::setCurrentState(RwaState *state)
{
    listModel->setStringList(backend->assetStringList);
}




