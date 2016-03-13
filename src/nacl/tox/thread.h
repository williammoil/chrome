#ifndef CHROME_GITHUB_COM_THREAD_H
#define CHROME_GITHUB_COM_THREAD_H

#include <tox/tox.h>
#include <tox/toxav.h>

namespace CToxTox {
    class Thread {
    public:
        Tox *tox_;
        ToxAV *toxav_;
        bool isConnected;
        bool isTerminated;

        Thread() : tox_(NULL), toxav_(NULL), isConnected(false), isTerminated(false) { }

        void Run() {
            {
                tox_iterate(tox_);
                //if (toxAV)
                //	toxav_iterate(toxAV);
            }

            uint32_t interval = tox_iteration_interval(tox_);
            usleep(interval * 1000);
        }

        void Abort() { isTerminated = true; }
    };
}

#endif
