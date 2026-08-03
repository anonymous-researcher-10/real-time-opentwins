#include "controllers/OtroController.h"

void OtroController::registerRoutes(crow::SimpleApp& app, OtroService& otroService) {
    CROW_ROUTE(app, "/otro").methods(crow::HTTPMethod::GET)([&otroService](){
        otroService.diHola();
        return crow::response(200, "Ruta de OtroController");
    });
}