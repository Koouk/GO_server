#include "server.hpp"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <ctime>
#include <thread>
#include <unistd.h>

#include "game.hpp"
#include "NetworkOperations.hpp"
#include "spdlog/spdlog.h"

void Thread_func(std::pair<int,int> players)
{

    spdlog::info("Players{} {}\n",players.first,players.second);
    Game* game = new Game();
    game->Initialize(players);
    game->Run();
    delete game;

    // currentGames--; z mutexem
}


bool Server::Initialize() {
    int on = 1;

    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(port_);
    sfd_ = socket(AF_INET, SOCK_STREAM, 0);
	if( sfd_ == -1 )
    {
        spdlog::info("Socket error {} \n",strerror(errno));
        return(EXIT_FAILURE);
    }
    if(setsockopt(sfd_, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) == -1)
    {
        spdlog::info("Setcoskopt error {} \n",strerror(errno));
        return(EXIT_FAILURE);

    }   
	if( bind(sfd_, (struct sockaddr*)&saddr, sizeof(saddr)) == -1 )
    {
        spdlog::info("Bind error{} \n",strerror(errno));
        return(EXIT_FAILURE);
    }
	if( listen(sfd_,connectionQueue_) == -1 )
    {
        spdlog::info("Listen error{} \n",strerror(errno));
        return(EXIT_FAILURE);
    }
    
    fdMax_ = sfd_;
	//FD_ZERO(&mainMask_);
    FD_ZERO(&mainMaskW_);
    FD_ZERO(&mainMaskR_);
    return(EXIT_SUCCESS);   
}



void Server::Run() {
    fd_set temp_rmask, temp_wmask;
    socklen_t slt;
    struct sockaddr_in c_addr;
    timeval timeout;
    int rc, cfd; 
    
    if( Initialize() == EXIT_FAILURE)
        return;
    while(1) { 
		temp_rmask = mainMaskR_;
		temp_wmask = mainMaskW_;
		FD_SET(sfd_, &temp_rmask);

		timeout.tv_sec = 5 * 60;
		timeout.tv_usec = 0;
		rc = select(fdMax_ + 1, &temp_rmask, &temp_wmask, (fd_set*)0, &timeout);
		if (rc == 0) {
		    spdlog::info("Timed out\n");
		    continue;
		}

		if (FD_ISSET(sfd_, &temp_rmask)) {
		    slt = sizeof(c_addr);
		    cfd = accept(sfd_, (struct sockaddr*)&c_addr, &slt);
		    spdlog::info("new connection: {}\n", inet_ntoa((struct in_addr)c_addr.sin_addr));
		    FD_SET(cfd, &mainMaskR_);
		    if (cfd > fdMax_) 
                fdMax_ = cfd;
            clients_.insert(cfd);
		}

		for (auto &i : clients_) { 
		    if(FD_ISSET(i,&temp_rmask)){
		        HandleRead(i);

		  }
		  else if (FD_ISSET(i, &temp_wmask)) {
                HandleWrite(i);
                
			}	
		}
        

        while(readyPlayers_.size() > 1)
        {
            auto player1 =  readyPlayers_.front();
            readyPlayers_.pop();
            auto player2 =  readyPlayers_.front();
            readyPlayers_.pop();
            CreateNewGame(player1, player2);
        }

	}
	close(sfd_);
}



void Server::HandleRead(int client)
{
    auto data = network::ReadData(client);
    if(data.Type == "lobby" && data.Data == "ready")
    {
        FD_CLR(client,&mainMaskR_);
        FD_SET(client,&mainMaskW_);
    }
}

void Server::HandleWrite(int client)
{
    readyPlayers_.push(client);
    FD_CLR(client,&mainMaskW_);

}

void Server::RemoveClient(int client)
{
    clients_.erase(client);
	FD_CLR(client, &mainMaskW_);
    FD_CLR(client, &mainMaskR_);
	if (client == fdMax_)
	while(fdMax_ > sfd_ && ! (FD_ISSET(fdMax_, &mainMaskR_)|| FD_ISSET(fdMax_, &mainMaskW_) ))
		fdMax_ -= 1;
}

void Server::CreateNewGame(int player1, int player2)
{

     std::srand(std::time(nullptr));
     if( rand() % 2 )
        std::swap(player1, player2);

    network::SendData("found", "white", player1);
    network::SendData("found", "black", player2);

    RemoveClient(player1);
    RemoveClient(player2);
    try{
        currentGames_++;   //mutex
        std::thread thr(Thread_func, std::pair<int,int>(player1,player2));
	    thr.detach();
    }catch (std::system_error &e)
    {
        spdlog::info("Could not create new thread:", e.what());
        close(player1);
        close(player2);
    }
}