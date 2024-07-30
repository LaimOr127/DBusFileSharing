#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

bool checkExtensionCompatibility(const std::string &filepath);

namespace ipc {
    struct EndpointInfo {
        std::string name;
        std::string executablePath;
        std::shared_ptr<std::vector<std::string>> acceptedFormats = nullptr;
    };

    using namespace std::literals::string_literals;
    constexpr const char *DEFAULT_CONFIG_PATH = "/etc/sharing/dbus-sharing.conf";

    class SharingService {
    public:
        explicit SharingService(const std::string &configPath = std::string { DEFAULT_CONFIG_PATH });

        std::vector<std::string> getEndpoints() const;
        EndpointInfo getEndpointInfoByName(const std::string &name) const;
        void routeFileToEndpoint(const std::string &endpointName, const std::string &filepath) const;

        void run() const;
    private:
        void launchEndpointService(const std::string &serviceName, const std::string &filepath) const;
        void readConfig(const std::string &configPath);
        std::unordered_map<std::string, EndpointInfo> m_endpoints;
        std::string m_receivedFilepath;
        std::string m_resultPath;

        std::shared_ptr<sdbus::IConnection> m_connection;
    };
}

/*
 * SharingService - это системный сервис, запускаемый при запуске
 * помогающий другим приложениям получать информацию о
 * приложениях, предоставляющих общий доступ к файлам.
 * Он должен каким-то образом получать информацию об исполняемых файлах, которые
 * отвечают за обработку файлов (как их вызывать, какие
 * типы файлов, которые они поддерживают, и т.д.) и отправляют данные в эти приложения.
 * Оно должно содержать список этих исполняемых файлов.
 */
