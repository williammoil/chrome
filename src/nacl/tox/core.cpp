#include "core.h"

using namespace CToxTox;

struct bootstrap_node {
    std::string ipv4;
    std::string ipv6;
    unsigned short port;
    std::string pgp_public_key;
};

Core::Core() {
    pthread_mutex_init(&lock_, NULL);
}

Core::~Core() {
    pthread_mutex_destroy(&lock_);
}

void friend_request_cb(Tox *tox, const uint8_t *public_key, const uint8_t *message, size_t length,
                       void *user_data) {
    fprintf(stderr, "Friend request\n");

    tox_friend_add_norequest(tox, public_key, NULL);

}

void friend_message_cb(Tox *tox, uint32_t friend_number, TOX_MESSAGE_TYPE type, const uint8_t *message,
                       size_t length, void *user_data) {
    fprintf(stderr, "Friend message %s\n", message);

    tox_friend_send_message(tox, friend_number, type, message, length, NULL);
}

void self_connection_status_cb(Tox *tox, TOX_CONNECTION connection_status, void *user_data) {
    switch (connection_status) {
        case TOX_CONNECTION_NONE:
            fprintf(stderr, "Offline\n");
            break;
        case TOX_CONNECTION_TCP:
            fprintf(stderr, "Online, using TCP\n");
            break;
        case TOX_CONNECTION_UDP:
            fprintf(stderr, "Online, using UDP\n");
            break;
    }
}

std::shared_ptr <Core> Core::Create() {
    fprintf(stderr, "Create\n");

    auto instance = std::shared_ptr<Core>(new Core);
    instance->CreateThread();

    return instance;
}

void Core::CreateThread() {
    pthread_create(&polling_thread_, NULL, Core::PollThread, this);
}

void *Core::PollThread(void *context) {
    static_cast<Core *>(context)->Init();

    return NULL;
}

void Core::Init() {
    fprintf(stderr, "Initializing Tox\n");

    Thread toxThread;
    this->toxThread_ = &toxThread;

    if (!Connect()) {
        Disconnect();
        //SetStatus(ID_STATUS_OFFLINE);

        fprintf(stderr, "Tox could not be initialized\n");

        return;
    }

    fprintf(stderr, "Tox initialized\n");

    toxThread.isConnected = false;

    BootstrapNodes();

    int maxRetries = TOX_MAX_DISCONNECT_RETRIES;

    while (!toxThread_->isTerminated) {
        CheckConnection(maxRetries);
        pthread_mutex_lock(&lock_);

        toxThread_->Run();

        pthread_mutex_unlock(&lock_);
    }

    fprintf(stderr, "Tox terminated\n");

    toxThread.isConnected = false;
    Disconnect();
}

Tox_Options *Core::GetToxOptions() {
    TOX_ERR_OPTIONS_NEW optionsError;
    Tox_Options *options = tox_options_new(&optionsError);

    if (optionsError != TOX_ERR_OPTIONS_NEW_OK) {
        if (optionsError == TOX_ERR_OPTIONS_NEW_MALLOC) {
            fprintf(stderr, "Tox options error: Unable to allocate enough memory\n");
        } else {
            fprintf(stderr, "Tox options error: Undefined error\n");
        }

        return NULL;
    }

    options->savedata_type = TOX_SAVEDATA_TYPE_NONE;

    // TODO: Add proxy and read settings
    options->udp_enabled = false;
    options->ipv6_enabled = true;

    return options;
}

bool Core::Connect() {
    Tox_Options *options = GetToxOptions();
    if (options == NULL)
        return false;

    TOX_ERR_NEW initError;
    toxThread_->tox_ = tox_new(options, &initError);

    if (initError != TOX_ERR_NEW_OK) {
        if (initError == TOX_ERR_NEW_NULL) {
            fprintf(stderr, "Tox connect error: Unexpected arguments\n");
        } else if (initError == TOX_ERR_NEW_MALLOC) {
            fprintf(stderr, "Tox connect error: Unable to allocate enough memory\n");
        } else if (initError == TOX_ERR_NEW_PORT_ALLOC) {
            fprintf(stderr, "Tox connect error: Unable to bind to a port\n");
        } else if (initError == TOX_ERR_NEW_PROXY_BAD_TYPE) {
            fprintf(stderr, "Tox connect error: Invalid proxy type\n");
        } else if (initError == TOX_ERR_NEW_PROXY_BAD_HOST) {
            fprintf(stderr, "Tox connect error: Invalid proxy host\n");
        } else if (initError == TOX_ERR_NEW_PROXY_BAD_PORT) {
            fprintf(stderr, "Tox connect error: Invalid proxy port\n");
        } else if (initError == TOX_ERR_NEW_PROXY_BAD_HOST) {
            fprintf(stderr, "Tox connect error: Invalid proxy type\n");
        } else if (initError == TOX_ERR_NEW_PROXY_NOT_FOUND) {
            fprintf(stderr, "Tox connect error: Proxy not found\n");
        } else if (initError == TOX_ERR_NEW_LOAD_BAD_FORMAT) {
            fprintf(stderr, "Tox connect error: Options bad format\n");
        } else if (initError == TOX_ERR_NEW_LOAD_ENCRYPTED) {
            fprintf(stderr, "Tox connect error: Options encrypted\n");
        } else {
            fprintf(stderr, "Tox connect error: Undefined error\n");
        }

        //ShowNotification(ToxErrorToString(initError), TranslateT("Unable to initialize Tox core"), MB_ICONERROR);

        tox_options_free(options);
        return false;
    }

    fprintf(stderr, "Tox created\n");

    // TODO: Add callbacks
    tox_callback_self_connection_status(toxThread_->tox_, self_connection_status_cb, NULL);
    tox_callback_friend_request(toxThread_->tox_, friend_request_cb, NULL);
    tox_callback_friend_message(toxThread_->tox_, friend_message_cb, NULL);

    // TODO: Use settings
    const char *name = "Echo Bot";
    tox_self_set_name(toxThread_->tox_, (uint8_t *) name, strlen(name), NULL);

    uint8_t tox_address_bin[TOX_ADDRESS_SIZE];
    tox_self_get_address(toxThread_->tox_, tox_address_bin);

    char tox_address_hex[TOX_ADDRESS_SIZE * 2 + 1];
    sodium_bin2hex(tox_address_hex, sizeof(tox_address_hex), tox_address_bin, sizeof(tox_address_bin));

    for (size_t i = 0; i < sizeof(tox_address_hex) - 1; i++) {
        tox_address_hex[i] = toupper(tox_address_hex[i]);
    }

    fprintf(stderr, "Tox ID: %s\n", tox_address_hex);

    uint8_t user_name[TOX_MAX_NAME_LENGTH] = {0};
    tox_self_get_name(toxThread_->tox_, user_name);

    fprintf(stderr, "Tox nickname: %s\n", user_name);

    uint8_t statusMessage[TOX_MAX_STATUS_MESSAGE_LENGTH] = {0};
    tox_self_get_status_message(toxThread_->tox_, statusMessage);

    fprintf(stderr, "Tox status message: %s\n", statusMessage);

    return true;
}

void Core::Disconnect() {
    if (toxThread_) {
        // TODO: Tox AV
        //if (toxThread->toxAV) {
        // Kill toxav
        //toxav_kill(toxThread->toxAV);
        //}

        if (toxThread_->tox_) {
            //CancelAllTransfers();

            if (toxThread_->tox_ != NULL) {
                // Kill tox
                tox_kill(toxThread_->tox_);

                toxThread_->tox_ = nullptr;
            }
        }


        toxThread_ = nullptr;
    }
}

void Core::TryConnect() {
    if (toxThread_ != NULL) {
        fprintf(stderr, "Trying to reconnect with network (attempt %u of %u)\n", retriesCount_ + 1,
                TOX_MAX_CONNECT_RETRIES);

        if (tox_self_get_connection_status(toxThread_->tox_) != TOX_CONNECTION_NONE) {
            toxThread_->isConnected = true;
            fprintf(stderr, "Successfuly connected with network\n");

            retriesCount_ = 0;
        }
        else if (retriesCount_++ == TOX_MAX_CONNECT_RETRIES - 1) {
            fprintf(stderr, "Max retries exceeded, unable to connect with network\n");

            // TODO
            //toxThread_->isTerminated = true;

            retriesCount_ = 0;
        }
    }
}

void Core::CheckConnection(int &maxRetries) {
    fprintf(stderr, "Checking connection with network\n");

    bool isOffline = tox_self_get_connection_status(toxThread_->tox_) == TOX_CONNECTION_NONE;

    fprintf(stderr, "Connection status is %s\n", isOffline ? "offline" : "online");

    TOX_ERR_GET_PORT errorPort;
    uint16_t udpPort = tox_self_get_udp_port(toxThread_->tox_, &errorPort);

    if (errorPort == TOX_ERR_GET_PORT_OK) {
        fprintf(stderr, "Bound to UDP port %hu\n", udpPort);
    }

    if (errorPort == TOX_ERR_GET_PORT_NOT_BOUND) {
        fprintf(stderr, "Not bound to any UDP port\n");
    }

    uint16_t tcpPort = tox_self_get_tcp_port(toxThread_->tox_, &errorPort);

    if (errorPort == TOX_ERR_GET_PORT_OK) {
        fprintf(stderr, "Bound to TCP port %hu\n", tcpPort);
    }

    if (errorPort == TOX_ERR_GET_PORT_NOT_BOUND) {
        fprintf(stderr, "Not bound to any TCP port\n");
    }

    if (!toxThread_ || !toxThread_->isConnected) {
        TryConnect();
    } else if (tox_self_get_connection_status(toxThread_->tox_) != TOX_CONNECTION_NONE) {
        if (maxRetries < TOX_MAX_DISCONNECT_RETRIES) {
            fprintf(stderr, "Restored connection with network\n");

            // Reset the count
            maxRetries = TOX_MAX_DISCONNECT_RETRIES;
        }
    } else {
        if (maxRetries == TOX_MAX_DISCONNECT_RETRIES) {
            maxRetries--;

            fprintf(stderr, "Lost connection with network\n");
        }
        else if (maxRetries % 50 == 0) {
            maxRetries--;

            fprintf(stderr, "Refresh bootstrap nodes\n");

            BootstrapNodes();
        }
        else if (!(--maxRetries)) {
            // Set the connection flag to false
            toxThread_->isConnected = false;

            fprintf(stderr, "Disconnected from network\n");
        }
    }
}

static const std::vector <bootstrap_node> &GetBootstrapNodes() {
    static std::vector <bootstrap_node> bootstrap_nodes = {{
                                                                   {"144.76.60.215", "2a01:4f8:191:64d6::1", 33445, "04119E835DF3E78BACF0F84235B300546AF8B936F035185E2A8E9E0A67C8924F"},
                                                                   {"23.226.230.47", "2604:180:1::3ded:b280", 33445, "A09162D68618E742FFBCA1C2C70385E6679604B2D80EA6E84AD0996A1AC8A074"},
                                                                   {"178.21.112.187", "2a02:2308::216:3eff:fe82:eaef", 33445, "4B2C19E924972CB9B57732FB172F8A8604DE13EEDA2A6234E348983344B23057"},
                                                                   {"195.154.119.113", "2001:bc8:3698:101::1", 33445, "E398A69646B8CEACA9F0B84F553726C1C49270558C57DF5F3C368F05A7D71354"},
                                                                   {"192.210.149.121", "", 33445, "F404ABAA1C99A9D37D61AB54898F56793E1DEF8BD46B1038B9D822E8460FAB67"},
                                                                   {"46.38.239.179", "", 33445, "F5A1A38EFB6BD3C2C8AF8B10D85F0F89E931704D349F1D0720C3C4059AF2440A"},
                                                                   {"178.62.250.138", "2a03:b0c0:2:d0::16:1", 33445, "788236D34978D1D5BD822F0A5BEBD2C53C64CC31CD3149350EE27D4D9A2F9B6B"},
                                                                   {"130.133.110.14", "2001:6f8:1c3c:babe::14:1", 33445, "461FA3776EF0FA655F1A05477DF1B3B614F7D6B124F7DB1DD4FE3C08B03B640F"},
                                                                   {"104.167.101.29", "", 33445, "5918AC3C06955962A75AD7DF4F80A5D7C34F7DB9E1498D2E0495DE35B3FE8A57"},
                                                                   {"205.185.116.116", "", 33445, "A179B09749AC826FF01F37A9613F6B57118AE014D4196A0E1105A98F93A54702"},
                                                                   {"198.98.51.198", "2605:6400:1:fed5:22:45af:ec10:f329", 33445, "1D5A5F2F5D6233058BF0259B09622FB40B482E4FA0931EB8FD3AB8E7BF7DAF6F"},
                                                                   {"80.232.246.79", "", 33445, "CF6CECA0A14A31717CC8501DA51BE27742B70746956E6676FF423A529F91ED5D"},
                                                                   {"108.61.165.198", "", 33445, "8E7D0B859922EF569298B4D261A8CCB5FEA14FB91ED412A7603A585A25698832"},
                                                                   {"212.71.252.109", "2a01:7e00::f03c:91ff:fe69:9912", 33445, "C4CEB8C7AC607C6B374E2E782B3C00EA3A63B80D4910B8649CCACDD19F260819"},
                                                                   {"194.249.212.109", "2001:1470:fbfe::109", 33445, "3CEE1F054081E7A011234883BC4FC39F661A55B73637A5AC293DDF1251D9432B"},
                                                                   {"185.25.116.107", "2a00:7a60:0:746b::3", 33445, "DA4E4ED4B697F2E9B000EEFE3A34B554ACD3F45F5C96EAEA2516DD7FF9AF7B43"},
                                                                   {"192.99.168.140", "2607:5300:100:200::::667", 33445, "6A4D0607A296838434A6A7DDF99F50EF9D60A2C510BBF31FE538A25CB6B4652F"},
                                                                   {"46.101.197.175", "2a03:b0c0:3:d0::ac:5001", 443, "CD133B521159541FB1D326DE9850F5E56A6C724B5B8E5EB5CD8D950408E95707"}
                                                           }};
    return bootstrap_nodes;
}

void Core::BootstrapNodes() {
    fprintf(stderr, "Refreshing bootstrap nodes\n");

    auto bootstrap_nodes = GetBootstrapNodes();

    // Refresh the tox bootstrap nodes
    for (auto i = 0; i < TOX_MAX_CONNECTIONS; ++i) {
        auto index = rand() % bootstrap_nodes.size();

        std::string host = bootstrap_nodes[index].ipv4;
        int port = bootstrap_nodes[index].port;
        auto hexKey = HexToBin(bootstrap_nodes[index].pgp_public_key.c_str());

        if (!host.empty()) {
            fprintf(stderr, "Bootstrapping %s %u (IPv4)\n", host.c_str(), port);

            BootstrapNode(host.c_str(), port, hexKey.data());
        }

        host = bootstrap_nodes[index].ipv6;

        if (!host.empty()) {
            fprintf(stderr, "Bootstrapping %s %u (IPv6)\n", host.c_str(), port);

            BootstrapNode(host.c_str(), port, hexKey.data());
        }
    }

    fprintf(stderr, "Bootstrap nodes refreshed\n");
}

void Core::BootstrapNode(const char *address, int port, const uint8_t *hexKey) {
    TOX_ERR_BOOTSTRAP errorBootstrap;

    if (!tox_bootstrap(toxThread_->tox_,
                       address,
                       port,
                       hexKey,
                       &errorBootstrap)) {
        if (errorBootstrap == TOX_ERR_BOOTSTRAP_NULL) {
            fprintf(stderr, "UDP bootstrap error: Missing host or missing pgp key\n");
        } else if (errorBootstrap == TOX_ERR_BOOTSTRAP_BAD_HOST) {
            fprintf(stderr, "UDP bootstrap error: Bad host\n");
        } else if (errorBootstrap == TOX_ERR_BOOTSTRAP_BAD_PORT) {
            fprintf(stderr, "UDP bootstrap error: Bad port\n");
        }
    } else {
        if (errorBootstrap == TOX_ERR_BOOTSTRAP_OK) {
            fprintf(stderr, "UDP successfully bootstrapped\n");
        }
    }

    if (!tox_add_tcp_relay(toxThread_->tox_,
                           address,
                           port,
                           hexKey,
                           &errorBootstrap)) {
        if (errorBootstrap == TOX_ERR_BOOTSTRAP_NULL) {
            fprintf(stderr, "TCP bootstrap error: Missing host or missing pgp key\n");
        } else if (errorBootstrap == TOX_ERR_BOOTSTRAP_BAD_HOST) {
            fprintf(stderr, "TCP bootstrap error: Bad host\n");
        } else if (errorBootstrap == TOX_ERR_BOOTSTRAP_BAD_PORT) {
            fprintf(stderr, "TCP bootstrap error: Bad port\n");
        }
    } else {
        if (errorBootstrap == TOX_ERR_BOOTSTRAP_OK) {
            fprintf(stderr, "TCP successfully bootstrapped\n");
        }
    }
}

std::vector <uint8_t> Core::HexToBin(std::string const &hex) {
    std::vector <uint8_t> bin(hex.size() / 2, 0);
    if (sodium_hex2bin(bin.data(), bin.size(), hex.data(), hex.size(), NULL,
                       NULL, NULL) != 0) {
        throw std::runtime_error("Error in Core::HexToBin(std::string)");
    }

    return bin;
}
