#include "board.hpp"
#include <utility>
#include <unordered_set>
#include <stack>
#include "spdlog/spdlog.h"

Board::Board()
{
    board_.resize(size_, std::vector<PlayerColor>(size_, PlayerColor::none));
    previousboard_ = board_;
};

bool Board::ProcessMove(std::pair<int,int> move, PlayerColor color)
{
    int row = move.first;
    int col = move.second;
   
    if(!CheckIndex(row,col)){
        return false;
    }
    
    if(board_[row][col] != PlayerColor::none){
        return false;
    }
    auto temp_board = board_;
    board_[row][col] = color;
    SolveCaptures(row, col, color);

    if(CheckRepetitions())
    {
        board_ = temp_board;
        return false;
    }
    if(CheckSuicide(row,col)){
         board_ = temp_board;
        return false;
    
    }

    previousboard_ = temp_board;
    
    return true;

}


std::pair<int, int> Board::GetResult()
{

    std::vector<std::vector<int>> checked_positions(board_.size(), std::vector<int>(board_.size(), -2));
    for (int r = 0; r < board_.size(); r++)
    {
        for (int c = 0; c < board_.size(); c++)
        {
            if (board_[r][c] == PlayerColor::black)
                checked_positions[r][c] = 0;
            else if (board_[r][c] == PlayerColor::white)
                checked_positions[r][c] = 1;
        }
    }
    for (int r = 0; r < board_.size(); r++)
    {
        for (int c = 0; c < board_.size(); c++)
        {
            if (checked_positions[r][c] == -2)
                CalculateUnknown(checked_positions, r, c);
        }
    }

    int black_area = 0;
    int white_area = 0;
    for(int r = 0; r < checked_positions.size(); r++) {
        for (int c = 0; c < checked_positions.size(); c++)
        {
            if(checked_positions[r][c] == 0)
                black_area++;
            if(checked_positions[r][c] == 1)
                white_area++;
        }
    }
    return {black_area, white_area};
}


bool Board::CheckSuicide(int row, int col)
{

    return Captures(row,col,false);
}


bool Board::CheckIndex(int row, int col)
{
    return row >= 0 && row < size_ && col >= 0 && col < size_;

}

bool Board::CheckRepetitions()
{
    for (int i = 0; i < board_.size(); i++)
    {
        for (int j = 0; j < board_.size(); j++)
        {
            if (board_[i][j] != previousboard_[i][j])
                return false;
        }
    }
    return true;
}

void Board::SolveCaptures(int rowB, int colB, PlayerColor color)
{

    for (auto dir : std::vector<std::pair<int, int>>{{0, 1}, {0, -1}, {1, 0}, {-1, 0}})
    {
        int row = rowB + dir.first;
        int col = colB + dir.second;
        if (CheckIndex(row, col) && board_[row][col] != PlayerColor::none && board_[row][col] != color)
            Captures(row, col, true);
    }
}
    

bool Board::Captures(int row, int col, bool remove)
{
    std::unordered_set<int> visited;
    std::vector<std::pair<int, int>> captured;
    std::stack<std::pair<int, int>> toVisit; 
    auto color = board_[row][col];

    visited.emplace((int)(row * size_ + col));
    captured.push_back( {row,col} );
    toVisit.emplace(row, col);
    while (!toVisit.empty())
    {
        auto point = toVisit.top();
        toVisit.pop();

        for (auto dir : std::vector<std::pair<int, int>>{{0, 1}, {0, -1}, {1, 0}, {-1, 0}})
        {
            int r = point.first + dir.first;
            int c = point.second + dir.second;
            if (CheckIndex(r, c) && !visited.count((int)((r * size_) + c)) && board_[r][c] == color)
            {
                toVisit.emplace(r, c);
                visited.emplace((int)(r * size_ + c));
                captured.push_back({r, c});
            }
            else if (CheckIndex(r, c) && board_[r][c] == PlayerColor::none){
                return false;
            }
        }
    }
    
    if(remove)
    for (auto point : captured)
    {
        board_[point.first][point.second] = PlayerColor::none;
    }
    return true;

}


void Board::CalculateUnknown(std::vector<std::vector<int>> &positions, int start_r, int start_c)
{

    std::vector<std::pair<int, int>> points;
    std::stack<std::pair<int, int>> stack;

    points.emplace_back(start_r, start_c);
    stack.emplace(start_r, start_c);

    bool white_neightbour = false;
    bool black_neightbour = false;
    while (!stack.empty())
    {
        auto point = stack.top();
        stack.pop();
        for (auto dir : std::vector<std::pair<int, int>>{{-1, 0}, {1, 0}, {0, -1}, {0, 1}})
        {
            int nr = point.first + dir.first;
            int nc = point.second + dir.second;
            if (CheckIndex(nr,nc))
            {
                if (positions[nr][nc] == -2)
                {
                    points.emplace_back(nr, nc);
                    positions[nr][nc] = -1;
                    stack.emplace(nr, nc);
                }
                else if (positions[nr][nc] == 0)
                    black_neightbour = true;
                else if (positions[nr][nc] == 1)
                    white_neightbour = true;
            }
        }
    }
    if (white_neightbour && !black_neightbour)
    {
        for (auto p : points)
        {
            positions[p.first][p.second] = 1;
        }
    }
    else if (!white_neightbour && black_neightbour)
    {
        for (auto p : points)
        {
            positions[p.first][p.second]  = 0;
        }
    }
}