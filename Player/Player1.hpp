#include <string>
#include "random.hpp"
#include "player.hpp"
#include "../include/pgeister.hpp"

#define PLAYOUT_COUNT 100

class Player1: public Player{
    cpprefjp::random_device rd;
    std::mt19937 mt;
    int depth = 7;
public:
    Player1(): mt(rd()) {
    }

    virtual std::string decideRed(){
        cpprefjp::random_device rd;
        std::mt19937 mt(rd());

        std::uniform_int_distribution<int> serector(0, pattern.size() - 1);
        return pattern[serector(mt)];
    }

    virtual std::string decideHand(std::string res){
        game.setState(res);
        
        auto legalMoves = game.getLegalMove1st();
        for (auto move : legalMoves) {
            if (BlueOnGoal(move.unit, move.direct)) {
                return move;
            }
        }

        PGeister pbasegame{game};
        PGeister pnextgame{};
        std::vector<double> treevalue(legalMoves.size(), 0.0);

        for(int index = 0 ; index < legalMoves.size() ; index++ ) {
            auto m = legalMoves[index];
            pnextgame = pbasegame;
            pnextgame.move(m);
            treevalue[index] = treeSearchMin(pnextgame, depth, -100000, 100000);
        }

        int maxindex = 0;
        for(int index = 1 ; index < legalMoves.size() ; index++ ) {
            if( treevalue[maxindex] < treevalue[index] ) maxindex = index;
        }
        return legalMoves[maxindex];
    }

    virtual std::vector<Hand> candidateHand(){
        return game.getLegalMove1st();
    }

    virtual bool BlueOnGoal(Unit unit, Direction direct) {
        if (unit.y() == 0 && ((unit.x() == 0 && direct == Direction::West) || (unit.x() == 5 && direct == Direction::East))) return 1;
        return 0;
    }

    virtual double treeSearchMax(PGeister game, int depth, double alpha, double beta) {
        Result r = game.result();
        //std::cout << "r = " << r << std::endl;
        //game.printBoard();
        //std::cout << game.takenCount(UnitColor::Blue) << game.takenCount(UnitColor::Red) << game.takenCount(UnitColor::blue) << game.takenCount(UnitColor::red) << std::endl;
        if(depth <= 0) {
            return 0.0;
        } else if( r == Result::Escape1st || r == Result::TakeBlue1st || r == Result::TakenRed1st ) {
            return 10000.0;
        } else if( r == Result::Escape2nd || r == Result::TakeBlue2nd || r == Result::TakenRed2nd ) {
            return -10000.0;
        }

        const PGeister pbasegame = game;
        PGeister pnextgame = pbasegame;
        std::vector<Hand> legalMoves = game.getLegalMove1st();
        double maxValue = -100000;
        for( auto &m : legalMoves ) {
            double tmp = 0.0;
            pnextgame.move(m);
            tmp = treeSearchMin(pnextgame, depth - 1, alpha, beta);
            if(tmp > alpha) {
                alpha = tmp;
            } else if(tmp >= beta) {
                return tmp;
            }

            if(tmp > maxValue) maxValue = tmp;
            pnextgame = pbasegame;
        }

        return maxValue;
    }

    virtual double treeSearchMin(PGeister game, int depth, double alpha, double beta) {
        Result r = game.result();
        //std::cout << "r = " << r << std::endl;
        //game.printBoard();
        //std::cout << game.takenCount(UnitColor::Blue) << game.takenCount(UnitColor::Red) << game.takenCount(UnitColor::blue) << game.takenCount(UnitColor::red) << std::endl;
        if(depth <= 0) {
            return 0.0;
        } else if( r == Result::Escape1st || r == Result::TakeBlue1st || r == Result::TakenRed1st ) {
            return 10000.0;
        } else if( r == Result::Escape2nd || r == Result::TakeBlue2nd || r == Result::TakenRed2nd ) {
            return -10000.0;
        }

        const PGeister pbasegame = game;
        PGeister pnextgame = pbasegame;
        std::vector<Hand> legalMoves = game.getLegalMove2nd();
        double minValue = 100000.0;
        for( auto &m : legalMoves ) {
            double tmp = 0.0;
            pnextgame.move(m);
            tmp = treeSearchMax(pnextgame, depth - 1, alpha, beta);
            if(tmp < beta) {
                beta = tmp;
            } else if(tmp <= alpha) {
                return tmp;
            }

            if(tmp < minValue) minValue = tmp;
            pnextgame = pbasegame;
        }

        return minValue;
    }
};