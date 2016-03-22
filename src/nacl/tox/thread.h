#ifndef CTOX_TOX_THREAD_H
#define CTOX_TOX_THREAD_H

#include <iostream>

#include <tox/tox.h>
#include <tox/toxav.h>

namespace CToxTox {

    class Thread {
    public:
        Tox *tox_;
        ToxAV *toxav_;
        bool isConnected;
        bool isTerminated;

        Thread();

        ~Thread();

        void Run();

        void Abort();
    };
}

#endif
