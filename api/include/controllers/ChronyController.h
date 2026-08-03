#pragma once
#include "external/crow_all.h"
#include "services/ChronyService.h"

class ChronyController {
public:
    static void registerRoutes(crow::SimpleApp& app, ChronyService& chronyService);
};