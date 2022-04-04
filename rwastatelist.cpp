#include "rwastatelist.h"

RwaStateList::RwaStateList(QWidget* parent, RwaScene *scene) :
    RwaListView(parent, scene)
{
    setDragDropMode(QAbstractItemView::DropOnly);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(this, SIGNAL(sendSelectedStates(QStringList)),
              backend, SLOT(receiveSelectedStates(QStringList)));

    connect(backend, SIGNAL(sendSelectedStates(QStringList)),
              this, SLOT(setSelectedStates(QStringList)));
}

void RwaStateList::ListWidgetEditEnd(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{   
    QString newName = reinterpret_cast<QLineEdit*>(editor)->text();
    if(currentState->objectName() != newName.toStdString())
    {
        currentState->setObjectName(newName.toStdString());
        emit sendCurrentScene(currentScene);
        emit sendWriteUndo("Renamed State");
    }
}

void RwaStateList::setSelectedStates(QStringList states)
{
    QList<QListWidgetItem *> items = this->findItems("*", Qt::MatchWildcard);
    foreach(QListWidgetItem *item, items)
    {
         item->setSelected(false);
    }

    foreach(QListWidgetItem *item, items)
    {
        foreach(QString state, states)
        {
            if(item->text() == state)
                item->setSelected(true);
        }
    }
}

QStringList RwaStateList::getSelectedStates()
{
    QList<QListWidgetItem *> items = this->selectedItems();
    QStringList states;
    foreach(QListWidgetItem *item, items)
        states << item->text();

    return states;
}

void RwaStateList::setCurrentState(RwaState *state)
{
    if(!state)
         return;

    currentState = state;

     if(QObject::sender() == this->backend)
     {
         QList<QListWidgetItem *> items = findItems(QString::fromStdString(currentState->objectName()), Qt::MatchExactly);
         if(!items.empty())
         {
             setCurrentItem(items.at(0));
             int row = QListWidget::row(currentItem());
             setCurrentRow(row);
             emit sendSelectedStates(getSelectedStates());
         }
     }
     else
         emit sendCurrentState(currentState);
}

void RwaStateList::setCurrentStateFromCurrentListItem()
{
    RwaState *state = nullptr;
    if(currentScene)
    {
        lastSelectedState = currentState;
        if(currentItem())
            state = currentScene->getState(currentItem()->text().toStdString());

        if(state)
            setCurrentState(state);
    }
}

void RwaStateList::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);
    setCurrentStateFromCurrentListItem();
}

void RwaStateList::mouseReleaseEvent(QMouseEvent *event)
{
    QListWidget::mouseReleaseEvent(event);
    if(currentScene)
         emit sendSelectedStates( getSelectedStates() );
}

void RwaStateList::update()
{
    clear();
    RwaState *state;
    if(currentScene != nullptr)
    {
        foreach (state, currentScene->getStates())
        {
            addItem2List(QString::fromStdString(state->objectName()));
        }
    }
}

void RwaStateList::keyPressEvent(QKeyEvent *event)
{
    QListWidget::keyPressEvent(event);
    switch (event->key())
    {
        case 16777219: // Qt::Key_Delete not working on OSX
            if(currentState->objectName() != "FALLBACK" && currentState->objectName() != "BACKGROUND")
            {
                RwaState *toDelete = currentState;
                takeItem(getSelectedIndex());
                setCurrentState(currentScene->states.front());
                emit deleteState(QString::fromStdString(toDelete->objectName()));
            }
            else
                qDebug() << "Fallback/Background States can not be deleted.";
            break;
        case Qt::Key_Down:
            setCurrentStateFromCurrentListItem();
            break;
        case Qt::Key_Up:
            setCurrentStateFromCurrentListItem();
            break;
        default:
            break;
     }
}



