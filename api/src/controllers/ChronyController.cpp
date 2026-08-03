#include "controllers/ChronyController.h"

void ChronyController::registerRoutes(crow::SimpleApp& app, ChronyService& chronyService) {
    CROW_ROUTE(app, "/configure").methods(crow::HTTPMethod::PATCH)([&chronyService](const crow::request& req){
        auto json_body = crow::json::load(req.body);

        if (!json_body) return crow::response(400, "Invalid JSON");
        if (json_body.t() != crow::json::type::Object)  return crow::response(400, "Expected a JSON object");

        ChronyConfigDTO config;

        if (json_body.has("server") && json_body["server"].t() == crow::json::type::String) config.server = json_body["server"].s();
        if (json_body.has("polling_interval") && json_body["polling_interval"].t() == crow::json::type::Number) config.polling_interval = json_body["polling_interval"].i();
        if (json_body.has("minpoll") && json_body["minpoll"].t() == crow::json::type::Number) config.minpoll = json_body["minpoll"].i();
        if (json_body.has("maxpoll") && json_body["maxpoll"].t() == crow::json::type::Number) config.maxpoll = json_body["maxpoll"].i();

        // for (const auto& item : json_body.keys()){
        //     auto valor = json_body[item];

        //     if (valor.t() == crow::json::type::String){
        //         std::cout << "Tipo de la variable " << item << " : String" << std::endl;
        //     }else if (valor.t() == crow::json::type::Number){
        //         std::cout << "Tipo de la variable " << item << " : Number" << std::endl;
        //     }
        // }

        std::string error;

        bool success =chronyService.updateConfiguration(config, error);
        
        if (!success) {
            return crow::response(500, "Failed to update configuration: " + error);
        }else{
            return crow::response(200, "Configuration updated successfully");
        }
    });

    CROW_ROUTE(app, "/restart").methods(crow::HTTPMethod::POST)([&chronyService](){
        chronyService.restartService();
        return crow::response(200, "Chrony service restarted successfully");
    });
}