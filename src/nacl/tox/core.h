#ifndef CTOX_TOX_CORE_H
#define CTOX_TOX_CORE_H

#include <iostream>
#include <string>
#include <vector>

#include <pthread.h>
#include <sodium/utils.h>
#include <nacl_io/nacl_io.h>
#include <tox/tox.h>

#include "settings.h"
#include "thread.h"

#define TOX_MAX_CONNECTIONS 5
#define TOX_MAX_CONNECT_RETRIES 300
#define TOX_MAX_DISCONNECT_RETRIES 300

namespace CToxTox {
    class Core : public std::enable_shared_from_this<Core> {
    public:
        static std::shared_ptr <Core> Create();

        ~Core();

    private:
        Core();

        Core(const Core &) = delete;

        void operator=(const Core &) = delete;

        void CreateThread();

        static void *PollThread(void *context);

        void Init();

        Thread *toxThread_;

        pthread_t polling_thread_;
        pthread_mutex_t lock_;

        std::vector <uint8_t> HexToBin(std::string const &hex);

        std::shared_ptr <CToxTox::Settings> Settings();

        int retriesCount_ = 0;

        Tox_Options *GetToxOptions();

        bool Connect();

        void Disconnect();

        void CheckConnection(int &retriesCount);

        void TryConnect();

        void BootstrapNodes();

        void BootstrapNode(const char *address, int port, const uint8_t *binKey);

    };
}

#endif