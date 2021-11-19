#ifndef BARRIER_H
#define BARRIER_H

#include <barrierdata.h>

#include <QObject>

class Barrier
{
public:
    Barrier(int count);
    void wait();
    QSharedPointer<BarrierData> *getBarrierData();
private:
    QSharedPointer<BarrierData> data;
};

#endif // BARRIER_H
