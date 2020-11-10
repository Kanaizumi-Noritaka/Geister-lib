#include <string>
#include "random.hpp"
#include "player.hpp"


class RandomPlayer2: public Player{
    cpprefjp::random_device rd;
    std::mt19937 mt;
public:
    RandomPlayer2(): mt(rd()){
    }

    virtual std::string decideRed(){
        cpprefjp::random_device rd;
        std::mt19937 mt(rd());

        std::uniform_int_distribution<int> serector(0, pattern.size() - 1);
        return pattern[serector(mt)];
    }

    virtual std::string decideHand(std::string res){
        game.setState(res);

        game = Geister(res);
        auto legalMoves = game.getLegalMove1st();
        
        for (auto move : legalMoves) {
            if (BlueOnGoal(move.unit, move.direct)) {
                return move;
            }
        }

        std::uniform_int_distribution<int> serector1(0, legalMoves.size() - 1);
        auto action = legalMoves[serector1(mt) % legalMoves.size()];
        return action;
    }

    virtual std::vector<Hand> candidateHand(){
        return game.getLegalMove1st();
    }

    virtual bool BlueOnGoal(Unit unit, Direction direct) {
        if (unit.y() == 0 && ((unit.x() == 0 && direct == Direction::West) || (unit.x() == 5 && direct == Direction::East))) return 1;
        return 0;
    }
};