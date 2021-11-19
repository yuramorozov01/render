#include "barrier.h"

Barrier::Barrier(int count):
    data(new BarrierData(count))
{

}

void Barrier::wait() {
    this->data->wait();
}

QSharedPointer<BarrierData> *Barrier::getBarrierData() {
    return &this->data;
}
