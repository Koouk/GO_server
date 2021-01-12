#pragma once

#include <unordered_set>
#include <deque>
#include <atomic>

class Server{

    public:
        Server(int port = 10024) : port_(port) {};
        ~Server() {}; 

        void Run();

    private:
        bool Initialize();
        void HandleRead(int client);
        void HandleWrite(int client);
        void RemoveClient(int client);
        void CreateNewGame(int player1, int player2);

        fd_set mainMask_,mainMaskR_, mainMaskW_;
        std::unordered_set <int> clients_, toRemove_, responed_; 
        std::deque  <int> readyPlayers_;

        int sfd_, port_, fdMax_;
        std::atomic <int> currentGames_;  
        const int connectionQueue_ = 5;
        const int maxSimultanousGames_ = 8;

};