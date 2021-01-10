#include "game.hpp"
#include "NetworkOperations.hpp"
#include "spdlog/spdlog.h"
#include "board.hpp"
#include <string>
#include <unistd.h>

void Game::Initialize(std::pair<int,int> players)
{
    players_ = {players.first,players.second};
    currentTurn_ = PlayerColor::white;
    board_ = new Board();
    gameStatus_ = true;
    pass_ = false;
    winner_ =  0;
}


void Game::Run()
{
    while(gameStatus_){

        UpdateGame();
    }
    FinalizeGame();

    close(players_[0]);
    close(players_[1]);
    delete board_;
}

void Game::UpdateGame()
{
    auto move = network::ReadData(players_[currentTurn_]);
    
    if(move.Type == "move")
        if(board_->ProcessMove())
        {
            pass_ = false;
            network::SendData("move","accepted",players_[currentTurn_]);
            network::SendData("move",move.Data,players_[currentTurn_ ^ 1]);
            currentTurn_ ^=1;
        }
        else
        {
            network::SendData("move","wrong",players_[currentTurn_]);
        }
        
    else if(move.Type == "button")
    {
        if(move.Data == "pass")
        {
            if(pass_) {
                gameStatus_ = false;
                network::SendData("end","pass",players_[currentTurn_]);
                network::SendData("end","pass",players_[currentTurn_ ^ 1]);
            }
            else
            {
                pass_ = true;
                network::SendData("button","accepted",players_[currentTurn_]);
                network::SendData("button","pass",players_[currentTurn_ ^ 1]);
            }
            currentTurn_ ^=1;
        }
        else if(move.Data == "resign")
        {
            gameStatus_ = false;
            winner_ = currentTurn_ ^ 1;
            winner_+= 1;
            network::SendData("end","resign",players_[currentTurn_]);
            network::SendData("end","resign",players_[currentTurn_ ^ 1]);
            
        }
    }
}


void Game::FinalizeGame()
{
    if(winner_)
    {   
        winner_ -= 1;
        SendResults("victory","resign",players_[winner_]);
        SendResults("defeat","resign",players_[(winner_ ) ^ 1]);

    }else {
            auto result = board_->GetResult();
            std::string data = std::to_string(result.first) + " " + std::to_string(result.second);
            if(result.first > result.second)
            {
                SendResults("victory",data,players_[PlayerColor::white]);
                SendResults("defeat",data,players_[PlayerColor::black]);
            }
            else if(result.first < result.second)
            {
                SendResults("defeat",data,players_[PlayerColor::white]);
                SendResults("victory",data,players_[PlayerColor::black]);
            }
            else{
                SendResults("draw",data,players_[PlayerColor::white]);
                SendResults("draw",data,players_[PlayerColor::black]);
            }
    }

}


void Game::SendResults(std::string type, std::string data, int client)   //to zrobic w watku przerywanym po x sekundach
{
    auto rcv = network::ReadData(client);
    while(rcv.Type != "request" && rcv.Data != "results")
    {
        rcv = network::ReadData(client);
    }
    network::SendData(type,data,client);
}
