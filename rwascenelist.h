#ifndef RWASCENELIST_H
#define RWASCENELIST_H

#include "rwalistview.h"


class RwaSceneList : public RwaListView
{
    Q_OBJECT
    void keyPressEvent(QKeyEvent *event) override;
    QStringList getSelectedScenes();
    void setCurrentSceneFromCurrentItem();
public:
    RwaSceneList(QWidget *parent, RwaScene *scene);

public slots:
    void setCurrentScene(RwaScene *currentScene) override;
    void update();

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void deleteScene(const QString &path);
    void sendSelectedScenes(QStringList scenes);
private slots:
    void ListWidgetEditEnd(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;
};

#endif // RWAGAMELIST_H
