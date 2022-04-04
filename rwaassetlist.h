#ifndef RWAASSETLIST_H
#define RWAASSETLIST_H

#include <QListWidget>
#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>
#include <QList>
#include <QDebug>
#include <QStringList>
#include "rwautilities.h"
#include "rwabackend.h"
#include "rwalistview.h"
#include <QFileInfo>
#include <QFile>

class RwaAssetList : public RwaListView
{
    Q_OBJECT

    public:
        RwaAssetList(QWidget *parent = nullptr, RwaScene *scene = nullptr);

public slots:
        int getNumberOfSelectedAssets();
        QStringList getSelectedAssets();

protected:
        void keyPressEvent(QKeyEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;
        void dropEvent(QDropEvent *event) override;
        void setCurrentAsset(RwaAsset1 *asset) override;
        void setCurrentState(RwaState *currentState) override;
        void add2ListAndCopy(QString fullpath);

    private:
        void setCurrentAssetFromCurrentItem();
signals:
        void deleteAsset(const QString &path);
        void newAsset(const QString &path);
        void newAsset(const QString &path, qint32 type);
        void sendSelectedAssets(QStringList assets);
protected slots:
    void ListWidgetEditEnd(QWidget *editor, QAbstractItemDelegate::EndEditHint hint) override;
};

#endif // RWAASSETLIST_H
