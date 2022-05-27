#ifndef RWAASSETLIST1_H
#define RWAASSETLIST1_H

#include "rwalistview1.h"

class RwaAssetList1 : public RwaListView1
{
public:
    RwaAssetList1(QWidget *parent);

public slots:
     void update();
     void receiveNewGameSignal();
     void setCurrentState(RwaState *state);

};

#endif // RWAASSETLIST1_H
