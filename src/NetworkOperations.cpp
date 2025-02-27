#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>

#include "json.hpp"
#include "spdlog/spdlog.h"
#include "NetworkOperations.hpp"



using nlohmann::json;

namespace network{

struct DataTemplate;
void to_json(json& j, const DataTemplate& p) {
        j = json{{"Type", p.Type}, {"Data", p.Data}};
    }

void from_json(const json& j, DataTemplate& p) {
    j.at("Type").get_to(p.Type);
    j.at("Data").get_to(p.Data);
}

bool SendData(std::string type, std::string message, int client)
{
    try{
    DataTemplate data {type, message};
    json j = data;
    std::string conv_json = j.dump() + "\n"; 
    int n = conv_json.size();
    char buffer[1024];
    for (int i = 0; i < sizeof(conv_json) || i < 1024; i++) 
        buffer[i] = conv_json[i];
    spdlog::info("Data to send: {} to {}",conv_json, client);
    int out = 0;
	do { 
		int sb = write(client, buffer + out, n - out );
        if(sb <= 0)
            return false;
		out += sb;
	}while( out < n );
    
    return true;
    }catch(const std::exception&){

        return false;
    }
}

DataTemplate ReadData(int client)
{
    
	int inp = 0;
	bool loop = true;
    char buffer[1024];
    DataTemplate error{"error","error"};

    try{
	while(loop) {
	int rb =  read(client, buffer + inp , 1024 -inp);
    if(rb <= 0)
    { 
        return error;
    }
	for( int in = inp; in<  inp + rb; in++)
    { 
		if(buffer[in] == '\n' ){
        	loop = false;	
            buffer[in]= '\0';	
        }	
	    
	}
    inp += rb;
    }

    std::string rec_data(buffer);
    json j = json::parse(rec_data);
    spdlog::info("Recieived data: {} from: {}",rec_data), client;
    return j.get<DataTemplate>();
    }catch(const std::exception&){

        return error;
    }
}

}