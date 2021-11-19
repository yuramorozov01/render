#ifndef BARRIERDATA_H
#define BARRIERDATA_H

#include <QMutex>
#include <QWaitCondition>
#include <QSharedPointer>

#include <QObject>

class BarrierData: public QObject
{
    Q_OBJECT
public:
    BarrierData(int count);
    void wait();
private:
    Q_DISABLE_COPY(BarrierData)
    int count;
    int currentCount;
    QMutex mutex;
    QWaitCondition condition;

signals:
    void passed();
};

#endif // BARRIERDATA_H
