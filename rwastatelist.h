#ifndef RWASTATELISTVIEW_H
#define RWASTATELISTVIEW_H

#include "rwalistview.h"

class RwaStateList : public RwaListView
{
    Q_OBJECT
    void keyPressEvent(QKeyEvent *event) override;
    QStringList getSelectedStates();
    void setCurrentStateFromCurrentListItem();
public:
    RwaStateList(QWidget *parent, RwaScene *scene);

public slots:
    void setCurrentState(RwaState *currentState) override;
    virtual void update() override;

    void setSelectedStates(QStringList states);
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

signals:
    void deleteState(const QString &path);
    void sendSelectedStates(QStringList states);
    void deleteSelectedStates(QStringList states);
protected slots:
    void ListWidgetEditEnd(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;
};

#endif // RWASTATELISTVIEW_H
