#ifndef SHARING_HPP
#define SHARING_HPP

#include <sdbus-c++/sdbus-c++.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

namespace ipc {

struct EndpointInfo {
    std::string name;
    std::string executablePath;
    std::shared_ptr<std::vector<std::string>> acceptedFormats;
};

class SharingService {
public:
    explicit SharingService(const std::string &configPath);
    void run() const;
    void routeFileToEndpoint(const std::string &endpointName, const std::string &filepath) const;
    std::vector<std::string> getEndpoints() const;
    EndpointInfo getEndpointInfoByName(const std::string &name) const;

private:
    void launchEndpointService(const std::string &serviceName, const std::string &filepath) const;
    std::unique_ptr<sdbus::IConnection> m_connection;
    std::unordered_map<std::string, EndpointInfo> m_endpoints;
};

} // namespace ipc

#endif // SHARING_HPP


/*
 * SharingService — это системный сервис, запускаемый при запуске.
 * помощь другим приложениям в получении информации о
 * приложения, обеспечивающие обмен файлами.
 * Он должен каким-то образом получить информацию об исполняемых файлах, которые
 * отвечают за работу с файлами (как их вызывать, какие
 * типы файлов, которые они поддерживают, и т. д.) и отправлять данные в эти приложения.
 * Он должен содержать список этих исполняемых файлов.
 */
