#define ASIO_STANDALONE
#include "external/crow_all.h"

#include "controllers/ChronyController.h"
#include "services/ChronyService.h"
#include "controllers/OtroController.h"
#include "services/OtroService.h"

int main(){
    crow::SimpleApp app;

    ChronyService chronyService;
    OtroService otroService;

    ChronyController::registerRoutes(app, chronyService);
    OtroController::registerRoutes(app, otroService);

    // Iniciar el servidor en el puerto 8080 con soporte multihilo
    std::cout << "Starting server on port 8080..." << std::endl;
    app.port(8080).run();
}