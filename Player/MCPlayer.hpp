#include <string>
#include "random.hpp"
#include "player.hpp"
#include "simulator.hpp"

#define PLAYOUT_COUNT 100

class MCPlayer: public Player{
    cpprefjp::random_device rd;
    std::mt19937 mt;
public:
    MCPlayer(): mt(rd()){
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

        std::vector<double> rewards(legalMoves.size(), 0.0);

        // 合法手の数だけ子局面を生成する
        std::vector<Simulator> children;
        for ( int i = 0; i < legalMoves.size(); i++ ) {
            auto m = legalMoves[i];
            Simulator sim = Simulator(game);
            sim.root.move(m);
            children.push_back(sim);
        }

        // 規定回数のプレイアウトを実行
        for (int i = 0; i < legalMoves.size(); i++) {
            rewards[i % legalMoves.size()] += children[i % legalMoves.size()].run(PLAYOUT_COUNT); // プレイアウトの実行と結果の加算（勝：＋１，敗：－１，引分：０）
        }

        int index_max = 0;
        for (int i = 1; i < legalMoves.size(); i++) {
            if(rewards[index_max] < rewards[i % legalMoves.size()]) index_max = i;
        }

        return legalMoves[index_max];
    }

    virtual std::vector<Hand> candidateHand(){
        return game.getLegalMove1st();
    }

    virtual bool BlueOnGoal(Unit unit, Direction direct) {
        if (unit.y() == 0 && ((unit.x() == 0 && direct == Direction::West) || (unit.x() == 5 && direct == Direction::East))) return 1;
        return 0;
    }
};