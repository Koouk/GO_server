#pragma once

#include <vector>

enum PlayerColor{
    white,
    black,
    none
};

class Game{

    public:
        Game() {};
        ~Game() {}; 

        void Run();
        void Initialize(std::pair<int,int> players);
    private:
        
        bool curretnTurn_;
        std::vector<int> players_;
};