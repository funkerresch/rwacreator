#include "rwalistview.h"

RwaListView::RwaListView(QWidget *parent, RwaScene *scene) :
     QListWidget(parent)
{
    this->backend = RwaBackend::getInstance();
    this->currentScene = scene;

    if(!scene->getStates().empty())
        this->currentState = scene->getStates().front();
    else
        this->currentState = nullptr;

    connect(this, SIGNAL(sendCurrentState(RwaState*)),
              backend, SLOT(receiveLastTouchedState(RwaState*)));

    connect(this, SIGNAL(sendCurrentScene(RwaScene*)),
            backend, SLOT(receiveLastTouchedScene(RwaScene*)));

    connect(this, SIGNAL(sendCurrentAsset(RwaAsset*)),
            backend, SLOT(receiveLastTouchedAsset(RwaAsset*)));

    connect(backend, SIGNAL(sendLastTouchedState(RwaState *)),
              this, SLOT(setCurrentState(RwaState*)));

    connect(backend, SIGNAL(sendLastTouchedScene(RwaScene *)),
              this, SLOT(setCurrentScene(RwaScene*)));

    connect(backend, SIGNAL(sendLastTouchedAsset(RwaAsset1 *)),
              this, SLOT(setCurrentAsset(RwaAsset1*)));

    connect(itemDelegate(), SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SLOT(ListWidgetEditEnd(QWidget*, QAbstractItemDelegate::EndEditHint)));
    this->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    connect(this, SIGNAL(sendWriteUndo(QString)), backend, SLOT(receiveWriteUndo(QString)));
}

void RwaListView::setUndoAction(const QString &value)
{
    undoAction = value;
    dirty = true;
}

void RwaListView::addItem2List(QString itemName)
{
    QListWidgetItem *item = new QListWidgetItem(itemName, this);
    if(itemName != "BACKGROUND" && itemName != "FALLBACK")
        item->setFlags( item->flags() | Qt::ItemIsEditable );
    addItem(item);
}

qint32 RwaListView::getSelectedIndex()
{
    if(selectedIndexes().isEmpty())
        return -1;
    else
        return this->selectedIndexes().first().row();
}

void RwaListView::mouseMoveEvent(QMouseEvent *event)
{
    QListWidget::mouseMoveEvent(event);
}

QStringList RwaListView::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list";
    return types;
}

void RwaListView::dragMoveEvent(QDragMoveEvent *event)
{
    qDebug("dragMoveEvent");
    //event->acceptProposedAction();
    QListWidget::dragMoveEvent(event);
}

void RwaListView::setCurrentState(RwaState *currentState)
{
    if(currentState == nullptr)
        return;

    this->currentState = currentState;
    int index = 0;
    foreach(RwaState *state, currentScene->getStates())
    {
        if(state == currentState)
            break;

        index++;
    }

    setCurrentRow(index);
}

void RwaListView::setCurrentState(qint32 stateNumber)
{
    RwaState *state;
    if(!currentScene)
        return;
    if(currentScene->getStates().empty())
        return;

    auto statesList = currentScene->getStates().begin();
    std::advance(statesList, stateNumber);

    state = *statesList;
    emit sendCurrentState(state);
}

void RwaListView::setCurrentAsset(RwaAsset1 *currentAsset)
{
    this->currentAsset = currentAsset;
}

void RwaListView::setCurrentScene(RwaScene *currentScene)
{
    this->currentScene = currentScene;
}

void RwaListView::setCurrentScene(qint32 sceneNumber)
{
    if(sceneNumber <= backend->getNumberOfScenes())
        setCurrentScene(backend->getSceneAt(sceneNumber));
}

void RwaListView::update()
{
   /* clear();
    qDebug() << "FKAéLKSJFAéLSKJFAéLSKDJ";
    RwaState *state;
    if(currentScene != NULL)
    {
        foreach (state, currentScene->getStates())
        {
            addItem2List(QString::fromStdString(state->objectName()));




        }

    }*/
}




void RwaListView::dropEvent(QDropEvent *event)
{


}
