#include "game.hpp"
#include "NetworkOperations.hpp"
#include "spdlog/spdlog.h"



void Game::Initialize(std::pair<int,int> players)
{
    players_ = {players.first,players.second};
    currentTurn_ = PlayerColor::white;

}


void Game::Run()
{

    

}

