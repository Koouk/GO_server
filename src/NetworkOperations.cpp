#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <future>
#include <thread>
#include <chrono>

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

bool SendData(std::string type, std::string message, int client, int timeout)
{
    try{
        DataTemplate data {type, message};
        json j = data;
        std::string conv_json = j.dump() + "\n"; 
        int n = conv_json.size();
        const char *buffer = conv_json.c_str();

        spdlog::info("Data to send: {} {}",conv_json, timeout);
        int out = 0;
	    do { 


            auto af = async(std::launch::async, write, client, buffer + out, n - out);
            auto status = af.wait_for(std::chrono::seconds(timeout));
            spdlog::info("StatusS {} ",status);
            if (status == std::future_status::timeout) {
                spdlog::info("TimeoutS");
                return false;
            } else if (status == std::future_status::ready) {
                out += af.get();
            }


	    //	int sb = write(client, buffer + out, n - out );
	    //	out += sb;
	    }while( out < n );
        return true;

    }catch(const std::exception& e){
            return false;
    }
}

DataTemplate ReadData(int client, int timeout)
{
    
	int inp = 0;
	bool loop = true;
    char buffer[1024];
    DataTemplate error = {"error","error"};
    try{
	while(loop) {
        auto af = async(std::launch::async, read, client, buffer + inp, 1024 - inp);
        auto status = af.wait_for(std::chrono::seconds(timeout));
        spdlog::info("StatusR {} ",status);
        if (status == std::future_status::timeout) {
            spdlog::info("TimeoutW");
            return error;
        } else if (status == std::future_status::ready) {      
            int rb = af.get();
            if(rb == 0 )
                return error;
	        for( int in = inp; in<  inp + rb; in++)
            { 
	        	if(buffer[in] == '\n' ){
                	loop = false;	
                    buffer[in]= '\0';	
                }	

	        }
             inp += rb;
             }      
        }
        std::string rec_data(buffer);
        json j = json::parse(rec_data);
        spdlog::info("Recieived data: {}",rec_data);
        return j.get<DataTemplate>();

    }catch(const std::exception& e){
        return error;
    }
}


}