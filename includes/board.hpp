#pragma once
#include <utility>
#include <vector>

enum PlayerColor{
    white,
    black,
    none
};

class Board {
    public:
    Board();
    bool ProcessMove(std::pair<int,int> move, PlayerColor color);
    std::pair<int, int> GetResult();

    private:
    void SolveCaptures(int row, int col, PlayerColor color);
    bool CheckIndex(int row, int col);
    bool CheckSuicide(int row, int col);
    bool Captures(int row, int col, bool remove );
    bool CheckRepetitions();
    void CalculateUnknown(std::vector<std::vector<int>> &positions, int start_r, int start_c);


    const int size_ = 9;
    std::vector<std::vector<PlayerColor>> board_, previousboard_;
    
};