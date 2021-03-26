#include "server.hpp"
#include <memory>

int main(int argc, char *argv[])
{
    std::unique_ptr<Server> server;
    if(argc > 0)
        server =  std::make_unique<Server>(atoi(argv[1]));
    else
    {
        server =std::make_unique<Server>();
    }
    server->Run();

    return 0;
}