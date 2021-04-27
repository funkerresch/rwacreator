#include "rwaassetmodel.h"

int RwaAssetModel::rowCount(const QModelIndex &parent) const
{
    return stringList.count();
}

QVariant RwaAssetModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= stringList.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return stringList.at(index.row());
        qDebug("select item");
    }
    else
        return QVariant();
}

QVariant RwaAssetModel::currentSelected(const QModelIndex &index)
{
    qDebug("clicked");
    return stringList.at(index.row());

}

QVariant RwaAssetModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString("Column %1").arg(section);
    else
        return QString("Row %1").arg(section);
}

bool RwaAssetModel::setData(const QModelIndex &index,
                              const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {

        stringList.replace(index.row(), value.toString());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool RwaAssetModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        stringList.insert(position, "");
    }

    endInsertRows();
    return true;
}

bool RwaAssetModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        stringList.removeAt(position);
    }

    endRemoveRows();
    return true;
}

Qt::DropActions RwaAssetModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags RwaAssetModel::flags(const QModelIndex &index) const
{
    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    else
        return Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
}

QStringList RwaAssetModel::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list";
    return types;
}

QMimeData *RwaAssetModel::mimeData(const QModelIndexList &indexes) const
{
    qDebug("mimeData");
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            QString text = data(index, Qt::DisplayRole).toString();
            stream << text;
        }
    }

    mimeData->setData("application/vnd.text.list", encodedData);
    return mimeData;
}

bool RwaAssetModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    qDebug("dropData");
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("text/uri-list"))
        return false;

    if (column > 0)
        return false;

    int beginRow;

    if (row != -1)
        beginRow = row;

    else if (parent.isValid())
        beginRow = parent.row();

    else
        beginRow = rowCount(QModelIndex());

    QString str = data->text();
    QStringList newItems = str.split( "\n", QString::SkipEmptyParts );

    int rows = 0;
    int remove = 0;

    foreach( const QString line, newItems )
    {
       QFileInfo info(line);
       if(!info.completeSuffix().compare("pd") )
           ++rows;
       else if(!info.completeSuffix().compare("wav") )
           ++rows;
       else if(!info.completeSuffix().compare("aif") )
           ++rows;

       else
       {
          qDebug("rmove");
          newItems.removeAt(remove++);
       }
    }

    insertRows(beginRow, rows, QModelIndex());

    foreach (const QString &text, newItems)
    {
        QModelIndex idx = index(beginRow, 0, QModelIndex());
        setData(idx, text);
        beginRow++;
    }

    return true;
}
