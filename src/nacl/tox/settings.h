#ifndef CTOX_TOX_SETTINGS_H
#define CTOX_TOX_SETTINGS_H

#include <string>

#include <tox/tox.h>

namespace CToxTox {
    class Settings : public std::enable_shared_from_this<Settings> {
    public:
        static std::shared_ptr<Settings> Create();

        bool get_use_udp();
        TOX_PROXY_TYPE get_proxy_type();
        //TOX_CONNECTION get_connection_type();
        std::string get_proxy_host();
        //std::vector <bootstrap_node> GetBootstrapNodes();
        int get_proxy_port();

    private:
        Settings();
        Settings(const Settings&) = delete;
        void operator=(const Settings&) = delete;

        // Properties
        bool property_use_udp_;
        TOX_PROXY_TYPE property_proxy_type_;
        //TOX_CONNECTION property_connection_type_();
        std::string property_proxy_host_;
        //std::vector <bootstrap_node> property_bootstrap_nodes_;
        int property_proxy_port_;
    };
}

#endif
