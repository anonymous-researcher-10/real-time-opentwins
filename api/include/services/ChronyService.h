#pragma once
#include <string>
#include <optional>

struct ChronyConfigDTO{
    std::optional<std::string> server;
    std::optional<int> polling_interval;
    std::optional<int> minpoll;
    std::optional<int> maxpoll;
};

class ChronyService {
public:
    // Devuelve true si tuvo éxito, y un mensaje de error si falló
    bool updateConfiguration(const ChronyConfigDTO& config, std::string& mensaje_error);
    void restartService();
};