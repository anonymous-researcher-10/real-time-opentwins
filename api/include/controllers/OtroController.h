#pragma once
#include "external/crow_all.h"
#include "services/OtroService.h"

class OtroController {
public:
    static void registerRoutes(crow::SimpleApp& app, OtroService& otroService);
};