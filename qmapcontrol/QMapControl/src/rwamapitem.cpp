#include "rwamapitem.h"

namespace qmapcontrol
{
    RwaMapItem::RwaMapItem(QPointF position, RwaLocation1 *rwaItem, int rwaType, QPixmap *pixmap, int channel) :
        QmapPoint(position.x(), position.y(), pixmap, rwaItem)
    {
        this->rwaItem = rwaItem;
        this->rwaType = rwaType;
        this->channel = channel;
    }

    RwaLocation1 *RwaMapItem::getRwaItem() const
    {
        return rwaItem;
    }

    int RwaMapItem::getRwaType() const
    {
        return rwaType;
    }

    int RwaMapItem::getChannel() const
    {
        return channel;
    }
}
