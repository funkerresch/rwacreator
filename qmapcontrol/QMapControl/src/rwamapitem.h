#ifndef RWAMAPITEM_H
#define RWAMAPITEM_H

#include "point.h"

namespace qmapcontrol
{
    class RwaMapItem : public QmapPoint
    {
    public:
        RwaMapItem(QPointF position, RwaLocation1 *rwaItem, int rwaType, QPixmap *pixmap, int channel = -1);

        RwaLocation1 *getRwaItem() const;
        int getRwaType() const;
        int getChannel() const;

    private:
        RwaLocation1 *rwaItem;
        int rwaType = 0;
        int channel = -1;
    };
}

#endif // RWAMAPITEM_H
