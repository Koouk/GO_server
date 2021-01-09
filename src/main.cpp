#include "server.hpp"


int main(int argc, char *argv[])
{
    Server* server;
    if(argc > 0)
        server = new Server(atoi(argv[1]));
    else
    {
        server = new Server();
    }
    
    server->Run();

    
    delete server;
    return 0;
}