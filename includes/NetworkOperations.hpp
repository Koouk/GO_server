#pragma once
#include "json.hpp"

using nlohmann::json;

namespace network{

struct DataTemplate ;
void to_json(json& j, const DataTemplate& p) ;
void from_json(const json& j, DataTemplate& p);
void SendData(std::string type, std::string message, int client);
DataTemplate ReadData(int client);


}