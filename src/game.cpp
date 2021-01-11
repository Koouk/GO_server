#include <string>
#include <unistd.h>
#include <future>
#include <thread>
#include <chrono>

#include "game.hpp"
#include "NetworkOperations.hpp"
#include "spdlog/spdlog.h"
#include "board.hpp"


void Game::Initialize(std::pair<int,int> players)
{
    players_ = {players.first,players.second};
    currentTurn_ = PlayerColor::black;
    board_ = new Board();
    gameStatus_ = true;
    pass_ = false;
    winner_ =  0;
    error_ = 0;
}
inline void Game::SetError(int player)
{
    gameStatus_ = false;
    error_ = std::min(player + 1,2);
    
}

void Game::Run()
{
    while(gameStatus_){

        UpdateGame();
    }
    FinalizeGame();

    close(players_[0]);
    close(players_[1]);
    spdlog::info("Koncze gre");
    delete board_;
}


void Game::UpdateGame()
{
    auto move = network::ReadData(players_[currentTurn_]);
    spdlog::info("Move {} ",move.Type);
    if(move.Type == "move")
        if(board_->ProcessMove(ToPair(move.Data), (PlayerColor)currentTurn_))
        {
            pass_ = false;
            if(!network::SendData("move","accepted",players_[currentTurn_]))
                SetError(currentTurn_);
            if(!network::SendData("move",move.Data,players_[currentTurn_ ^ 1]))
                SetError(currentTurn_ ^ 1);

            spdlog::info("wyslano");
            currentTurn_ ^=1;
        }
        else
        {
            if(!network::SendData("move","wrong",players_[currentTurn_]))
                SetError(currentTurn_);
        }
        
    else if(move.Type == "button")
    {
        if(move.Data == "pass")
        {
            if(pass_) {
                gameStatus_ = false;
                if(!network::SendData("end","pass",players_[currentTurn_]))
                    SetError(currentTurn_);
                if(!network::SendData("end","pass",players_[currentTurn_ ^ 1]))
                    SetError(currentTurn_ ^ 1);
            }
            else
            {
                pass_ = true;
                if(!network::SendData("button","accepted",players_[currentTurn_]))
                    SetError(currentTurn_);
                if(!network::SendData("button","pass",players_[currentTurn_ ^ 1]))
                    SetError(currentTurn_ ^ 1);
            }
            currentTurn_ ^=1;
        }
        else if(move.Data == "resign")
        {
            gameStatus_ = false;
            winner_ = currentTurn_ ^ 1;
            winner_+= 1;
            if(!network::SendData("end","resign",players_[currentTurn_]))
                SetError(currentTurn_);
            if(!network::SendData("end","resign",players_[currentTurn_ ^ 1]))
                SetError(currentTurn_ ^ 1);
            
        }
    }
    else if(move.Type == "error")
    {
        error_ = currentTurn_ + 1;
        gameStatus_ = false;
        network::SendData("end","error",players_[currentTurn_ ^ 1]);
        network::SendData("end","error",players_[currentTurn_ ]);
        spdlog::info("Wyslalem dane o bledzie{}",currentTurn_ ^ 1);
    }
}




void Game::FinalizeGame()
{
    if(error_)
    {
        if(error_ == 1) {
        spdlog::info("Wysylam dane o bledzie{}, {}",players_[0], players_[1]);
        SendResults("error","opponent",players_[0]);
        SendResults("error","error",players_[1]);
        }
        else
        {
        SendResults("error","opponent",players_[1 ]);
        SendResults("error","error",players_[0]);
        }
        
        return;

    }
    if(winner_)
    {   
        winner_ -= 1;
        SendResults("victory","resign",players_[winner_]);
        SendResults("defeat","resign",players_[ (winner_^ 1 ) ]);

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
    while(rcv.Type != "request" && rcv.Data != "results" && rcv.Type != "error")
    {
        rcv = network::ReadData(client);
    }
    network::SendData(type,data,client);
}


std::pair<int,int> Game::ToPair(std::string move)
{
    std::string delimiter = " ";
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = move.find (delimiter, pos_start)) != std::string::npos) {
        token = move.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (move.substr (pos_start));
    for(auto i : res)
     
    if(res.size() > 1)
    {
        return {std::stoi( res[0] ), std::stoi( res[1] )};
    }
    return {-1, -1};
}