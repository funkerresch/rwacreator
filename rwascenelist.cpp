#include "rwascenelist.h"

RwaSceneList::RwaSceneList(QWidget* parent, RwaScene *scene) :
    RwaListView(parent, scene)
{
    connect(this, SIGNAL(sendCurrentScene(RwaScene*)),
              backend, SLOT(receiveLastTouchedScene(RwaScene*)));

    connect(this, SIGNAL(sendSelectedScenes(QStringList)),
              backend, SLOT(receiveSelectedScenes(QStringList)));

    disconnect(backend, SIGNAL(sendLastTouchedState(RwaState *)),
              this, SLOT(setCurrentState(RwaState*)));

    connect(backend, SIGNAL(newGameLoaded()), this, SLOT(update()));
}

void RwaSceneList::ListWidgetEditEnd(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    QString newName = reinterpret_cast<QLineEdit*>(editor)->text();
    if(currentScene->objectName() != newName.toStdString())
    {
        currentScene->setName(newName.toStdString());
        setCurrentScene(currentScene);
        emit sendWriteUndo("Renamed Scene");
    }
}

QStringList RwaSceneList::getSelectedScenes()
{
    QList<QListWidgetItem *> items = this->selectedItems();
    QStringList scenes;
    foreach(QListWidgetItem *item, items)
        scenes << item->text();

    return scenes;
}

void RwaSceneList::setCurrentScene(RwaScene *currentScene)
{
    if(!currentScene)
         return;

     this->currentScene = currentScene;

     if(QObject::sender() == this->backend)
     {
         QList<QListWidgetItem *> items = findItems(QString::fromStdString(currentScene->objectName()), Qt::MatchExactly);
         if(!items.empty())
         {
             setCurrentItem(items.at(0));
             int row = QListWidget::row(currentItem());
             setCurrentRow(row);
             //qDebug() << "SET CURRENT ROW" << currentScene->objectName() << row;
             emit sendSelectedScenes( getSelectedScenes() );
         }

     }
     if(!(QObject::sender() == this->backend))
     {
         //qDebug() << "SEND CURRENT";
         emit sendCurrentScene(this->currentScene);
     }
}

void RwaSceneList::setCurrentSceneFromCurrentItem()
{
    RwaScene *scene = NULL;

    if(currentItem())
        scene = backend->getScene(currentItem()->text());

    if(scene)
        setCurrentScene(scene);

    emit sendSelectedScenes(getSelectedScenes());
}

void RwaSceneList::mousePressEvent(QMouseEvent *event)
{
    QListWidget::mousePressEvent(event);
    setCurrentSceneFromCurrentItem();
}

void RwaSceneList::keyPressEvent(QKeyEvent *event)
{
    QListWidget::keyPressEvent(event);
    switch (event->key())
      {
          case Qt::Key_Return:
          case Qt::Key_Enter:
              qDebug() << "Enter";
              break;
          case Qt::Key_Escape:
              qDebug() << "Escape";
              break;
          case Qt::Key_Insert:
              qDebug() << "Insert";
              break;
          case 16777219:
             // qDebug() << "Delete Scene";
              takeItem(getSelectedIndex());
              emit deleteScene(QString::fromStdString(currentScene->objectName()));
              break;
          case 16777237:
              setCurrentSceneFromCurrentItem();
              break;
          case 16777235:
              setCurrentSceneFromCurrentItem();
              break;
          default:
              qDebug() << "other" << event->key();
              break;
     }
}

void RwaSceneList::update()
{
    clear();
    RwaScene *scene;
    if(backend != nullptr)
    {
        foreach (scene, backend->getScenes())
        {
            addItem2List(QString::fromStdString(scene->objectName()));
        }

    }
}
