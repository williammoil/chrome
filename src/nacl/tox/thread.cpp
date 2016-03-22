#include "thread.h"

using namespace CToxTox;

Thread::Thread() : tox_(NULL), toxav_(NULL), isConnected(false), isTerminated(false) { }

Thread::~Thread() { }

void Thread::Run() {
    tox_iterate(tox_);
    //if (toxAV)
    //	toxav_iterate(toxAV);

    uint32_t interval = tox_iteration_interval(tox_);
    usleep(interval * 1000);
}

void Thread::Abort() {
    isTerminated = true;
}
