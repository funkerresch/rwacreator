#ifndef RWAASSETMODEL_H
#define RWAASSETMODEL_H

#include <QAbstractListModel>
#include <QStringListModel>
#include <QMimeData>
#include <QFileInfo>

class RwaAssetModel : public QAbstractListModel
{
    Q_OBJECT
public:
    RwaAssetModel(const QStringList &strings, QObject *parent = 0)
        : QAbstractListModel(parent), stringList(strings) {


    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role) const;
        QVariant headerData(int section, Qt::Orientation orientation,
                            int role = Qt::DisplayRole) const;


        Qt::ItemFlags flags(const QModelIndex &index) const;
        Qt::DropActions supportedDropActions() const;

        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
        bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
        bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());

        QStringList mimeTypes() const;

        QMimeData *mimeData(const QModelIndexList &indexes) const;

        bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

public slots:
        QVariant currentSelected(const QModelIndex &index);

    private:
        QStringList stringList;
 };



#endif // RWAASSETMODEL_H
