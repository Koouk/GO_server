#pragma once

#include <string>
#include <vector>
#include "board.hpp"



class Game{

    public:
        Game() {};
        ~Game() {}; 

        void Run();
        void Initialize(std::pair<int,int> players);
    private:
        void UpdateGame();
        void FinalizeGame();
        void SendResults(std::string type, std::string data, int client);

        Board* board_;
        bool currentTurn_, gameStatus_, pass_ ;
        int winner_;
        std::vector<int> players_;
};