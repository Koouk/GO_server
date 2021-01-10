#pragma once
#include <utility>

enum PlayerColor{
    white,
    black,
    none
};

class Board {
    public:
    bool ProcessMove();
    std::pair<int, int> GetResult();

    private:

    
};