#include "settings.h"

using namespace CToxTox;

Settings::Settings() {
    property_use_udp_ = false;
    property_proxy_type_ = TOX_PROXY_TYPE_NONE;
    property_proxy_host_ = "";
    property_proxy_port_ = 0;
}

bool Settings::get_use_udp() {
    return property_use_udp_;
}

TOX_PROXY_TYPE Settings::get_proxy_type() {
    return property_proxy_type_;
}

std::string Settings::get_proxy_host() {
    return property_proxy_host_;
}

int Settings::get_proxy_port() {
    return property_proxy_port_;
}

std::shared_ptr<Settings> Settings::Create() {
    return std::shared_ptr<Settings>(new Settings());
}