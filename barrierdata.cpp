#include "barrierdata.h"

BarrierData::BarrierData(int count) :
    count(count), currentCount(count)
{

}

void BarrierData::wait() {
    this->mutex.lock();
    this->currentCount--;
    if (this->currentCount > 1) {
        this->condition.wait(&this->mutex);
    } else if (this->currentCount == 1) {
        emit this->passed();
        this->condition.wait(&this->mutex);
    } else {
        this->currentCount = this->count;
        this->condition.wakeAll();
    }
    this->mutex.unlock();
}
