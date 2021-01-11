#pragma once
#include "json.hpp"

using nlohmann::json;

namespace network{

struct DataTemplate {
    std::string Type;
    std::string Data;
    };
void to_json(json& j, const DataTemplate& p) ;
void from_json(const json& j, DataTemplate& p);
bool SendData(std::string type, std::string message, int client, int timeout = 10);
DataTemplate ReadData(int client, int timeout = 90);


}