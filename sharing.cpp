#include "sharing.hpp"

#include <sdbus-c++/sdbus-c++.h>
#include <systemd/sd-journal.h>
#include <ini.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <stdio.h>
#include <sstream>
#include <algorithm>

constexpr char LIST_SEP = ','; // Константа для разделителя в списках форматов файлов

// Функция для проверки совместимости формата файла с поддерживаемыми форматами конечной точки
bool checkExtensionCompatibility(const ipc::EndpointInfo &info, const std::string &filepath) {
    const std::string actualFmt = filepath.substr(filepath.rfind('.'));
    return std::find(info.acceptedFormats->cbegin(), info.acceptedFormats->cend(), actualFmt) != info.acceptedFormats->cend();
}

// Обработчик для парсера ini-файла
static int iniHandler(void* user, const char* section, const char* name, const char* value) {
    auto* endpoints = static_cast<std::unordered_map<std::string, ipc::EndpointInfo>*>(user);
    std::string appName(section);
    if (endpoints->find(appName) == endpoints->end()) {
        (*endpoints)[appName] = ipc::EndpointInfo{appName, "", std::make_shared<std::vector<std::string>>()};
    }
    ipc::EndpointInfo& info = (*endpoints)[appName];
    if (std::string(name) == "formats") {
        std::string formats(value);
        std::stringstream ss(formats);
        std::string format;
        while (std::getline(ss, format, LIST_SEP)) {
            info.acceptedFormats->emplace_back(format);
        }
    } else if (std::string(name) == "cmd") {
        info.executablePath = value;
    }
    return 1;
}

// Конструктор класса SharingService
ipc::SharingService::SharingService(const std::string &configPath) {
    if (ini_parse(configPath.c_str(), iniHandler, &m_endpoints) < 0) {
        char errMsg[256];
        ::sprintf(errMsg, "File sharing proxy service could not open file %s: no such file or directory.", configPath.c_str());
        ::sd_journal_print(LOG_CRIT, errMsg);
        std::cerr << errMsg << '\n';
        throw std::runtime_error("Failed to load config file");
    }

    constexpr const char *sharingConnectionName = "org.rt.sharing";
    m_connection = sdbus::createSessionBusConnection(sharingConnectionName);

    constexpr const char *objPath = "/org/rt/router";
    constexpr const char *interfaceName = "org.rt.SharingService";
    auto router = sdbus::createObject(*m_connection, objPath);
    auto routeFunc = [this](const std::string &endpointName, const std::string &filepath) -> void {
        this->routeFileToEndpoint(endpointName, filepath);
    };
    auto getEndpointsFunc = [this, &router]() {
        const auto endpointsList = this->getEndpoints();
        router->emitSignal("endpointsReady").onInterface(interfaceName).withArguments(endpointsList);
        return endpointsList;
    };
    auto getEndpointFormats = [this](const std::string &endpointName) -> std::vector<std::string> {
        return *this->getEndpointInfoByName(endpointName).acceptedFormats;
    };

    // Регистрация методов
    router->registerMethod("getEndpoints").onInterface(interfaceName).implementedAs(std::move(getEndpointsFunc));
    router->registerMethod("passFileForProcessing").onInterface(interfaceName).implementedAs(std::move(routeFunc));
    router->registerMethod("getEndpointFormats").onInterface(interfaceName).implementedAs(std::move(getEndpointFormats));
    router->registerSignal("endpointsReady").onInterface(interfaceName).withParameters<std::vector<std::string>>();
    router->finishRegistration();

    m_connection->enterEventLoop(); // Вход в основной цикл обработки событий D-Bus
}

// Запуск основного цикла обработки событий D-Bus
void ipc::SharingService::run() const {
    m_connection->enterEventLoop();
}

// Маршрутизация файла на обработка конечной точки
void ipc::SharingService::routeFileToEndpoint(const std::string &endpointName, const std::string &filepath) const {
    try {
        this->launchEndpointService(endpointName, filepath);
    } catch (const std::out_of_range&) {
        char errMsg[256];
        ::sprintf(errMsg, "Endpoint service %s not found.", endpointName.c_str());
        ::sd_journal_print(LOG_ERR, errMsg);
        std::cerr << errMsg << '\n';
    } catch (const std::exception &e) {
        char errMsg[256];
        ::sprintf(errMsg, "Failed to route file to endpoint service %s: %s.", endpointName.c_str(), e.what());
        ::sd_journal_print(LOG_ERR, errMsg);
        std::cerr << errMsg << '\n';
    }
}

// Метод для получения списка конечных точек
std::vector<std::string> ipc::SharingService::getEndpoints() const {
    std::vector<std::string> res;
    for (const auto &[endpointName, _] : m_endpoints) {
        res.push_back(endpointName);
    }
    return res;
}

// Получения информации о конечной точке по имени
ipc::EndpointInfo ipc::SharingService::getEndpointInfoByName(const std::string &name) const {
    try {
        return m_endpoints.at(name);
    } catch (const std::out_of_range&) {
        char errMsg[256];
        ::sprintf(errMsg, "Endpoint service %s not found.", name.c_str());
        ::sd_journal_print(LOG_ERR, errMsg);
        std::cerr << errMsg << '\n';
        throw;
    }
}

// Запуска сервиса конечной точки
void ipc::SharingService::launchEndpointService(const std::string &serviceName, const std::string &filepath) const {
    const auto endpoint = m_endpoints.at(serviceName);
    const auto &endpointExecPath = endpoint.executablePath;
    if (checkExtensionCompatibility(endpoint, filepath)) {
        ::execl(endpointExecPath.c_str(), endpointExecPath.c_str(), filepath.c_str(), nullptr);
    } else {
        char errMsg[256];
        ::sprintf(errMsg, "File format of %s is not supported by endpoint service %s.", filepath.c_str(), serviceName.c_str());
        ::sd_journal_print(LOG_ERR, errMsg);
        std::cerr << errMsg << '\n';
    }
}
