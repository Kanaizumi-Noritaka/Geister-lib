#include <string>
#include "random.hpp"
#include "player.hpp"
#include "../include/mysimulator.hpp"

#define PLAYOUT_COUNT 50

class Player2: public Player{
    cpprefjp::random_device rd;
    std::mt19937 mt;
    std::vector<int> numbers = { };
    std::vector<double> rewards = { };
    int allRepeatNumber = 0;
public:
    Player2(): mt(rd()){
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

        std::vector<int> tmpnum(legalMoves.size(), 0);
        std::vector<double> tmprew(legalMoves.size(), 0);
        numbers = tmpnum;
        rewards = tmprew;
       
        // 合法手の数だけ子局面の動的確保をする
        std::vector<MySimulator*> children;
        for ( int i = 0; i < legalMoves.size(); i++ ) {
            auto m = legalMoves[i];
            MySimulator *sim = new MySimulator(game);
            sim -> root.move(m);
            children.push_back(sim);
        }

        std::cout << "Size : " << legalMoves.size() << std::endl;
        // 規定回数のプレイアウトを実行
        for (int i = 0; i < PLAYOUT_COUNT; i++) {
            int next = selectNext();
            double r = MinMonteCarloTreeSearching(children[next]);
            add(next, r);
            allRepeatNumber += 1;
        }

        int index_max = 0;
        for (int i = 1; i < legalMoves.size(); i++) {
            if(rewards[index_max] < rewards[i % legalMoves.size()]) index_max = i;
        }

        for( int j = 0; j < legalMoves.size(); j++ ) {
            delete children[j];
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

    virtual std::vector<int> findAllIndex() const {
        std::vector<int> ret = { };
        for( int i = 0; i <= numbers.size(); i++ ) {
            if(numbers[i] == 0) ret.push_back(i);
        }
        return ret;
    }

    virtual int ucbSelect() {
        std::vector<double> ucb(numbers.size(), 0.0);
        for( int i = 0; i < ucb.size(); i++ ) {
            ucb[i] += rewards[i] / numbers[i];
            ucb[i] += sqrt( log(allRepeatNumber) / numbers[i] );
        }

        int maxindex = 0;
        for(int j = 1; j < ucb.size(); j++) {
            if( ucb[maxindex] < ucb[j] ) maxindex = j;
        }
        return maxindex;
    }

    virtual int selectNext() {
        std::vector<int> t = findAllIndex();
        if(t.size() == 0) {
            return ucbSelect();
        } else {
            static std::uniform_int_distribution<> selector;
            selector.param(std::uniform_int_distribution<>::param_type(0, t.size() - 1));
            return selector(mt);
        }
    }

    virtual void add(int index, int value) {
        numbers[index] += 1;
        rewards[index] += value;
    }

    double MaxMonteCarloTreeSearching (MySimulator *ms) {
        Result r = ms -> root.result();
        if( r == Result::Escape1st || r == Result::TakeBlue1st || r == Result::TakenRed1st ) return 100.0;
        else if ( r == Result::Escape2nd || r == Result::TakeBlue2nd || r == Result::TakenRed2nd ) return -100.0;

        if( ms -> getReachNumber() >= 5 && ms -> nextSimulators.size() == 0 ) {
            auto legalMoves = ms -> root.getLegalMove1st();

            for ( int i = 0; i < legalMoves.size(); i++ ) {
                auto m = legalMoves[i];
                MySimulator *sim = new MySimulator(ms -> root);
                sim -> root.move(m);
                ms -> nextSimulators.push_back(sim);
            }

            std::vector<int> tmpnum(legalMoves.size(), 0);
            std::vector<double> tmprew(legalMoves.size(), 0);
            ms -> numbers = tmpnum;
            ms -> rewards = tmprew;
        }

        if( ms -> nextSimulators.size() == 0 ) { 
            ms -> reachNumber += 1;
            return ms -> run(1);
        }

        int next = ms -> selectNext();
        MySimulator *nms = ms -> nextSimulators[next];
        double reward = MinMonteCarloTreeSearching(nms);
        ms -> add(next, reward);
        return reward;
    }

    double MinMonteCarloTreeSearching (MySimulator *ms) {
        printf("%s\n", "Tree");
        std::cout << ms -> reachNumber << std::endl;
        std::cout << ms -> allRepeatNumber << std::endl;
        fflush(stdout);
        Result r = ms -> root.result();
        if( r == Result::Escape1st || r == Result::TakeBlue1st || r == Result::TakenRed1st ) return 100.0;
        else if ( r == Result::Escape2nd || r == Result::TakeBlue2nd || r == Result::TakenRed2nd ) return -100.0;

        if( ms -> getReachNumber() >= 5 && ms -> nextSimulators.size() == 0 ) {
            auto legalMoves = ms -> root.getLegalMove2nd();

            for ( int i = 0; i < legalMoves.size(); i++ ) {
                auto m = legalMoves[i];
                MySimulator *sim = new MySimulator(ms -> root);
                sim -> root.move(m);
                ms -> nextSimulators.push_back(sim);
            }

            std::vector<int> tmpnum(legalMoves.size(), 0);
            std::vector<double> tmprew(legalMoves.size(), 0.0);
            ms -> numbers = tmpnum;
            ms -> rewards = tmprew;
        }

        printf("%s\n", "Tree");
        fflush(stdout);

        if( ms -> nextSimulators.size() == 0 ) { 
            ms -> reachNumber += 1;
            ms -> allRepeatNumber += 1;
            return ms -> run(1);
        }

        int next = ms -> selectNext();
        printf("%s\n", "Tree");
        fflush(stdout);
        MySimulator *nms = ms -> nextSimulators[next];
        printf("%s\n", "Tree");
        fflush(stdout);
        double reward = MaxMonteCarloTreeSearching(nms);
        printf("%s\n", "Tree");
        fflush(stdout);
        ms -> add(next, reward);
        printf("%s\n", "Tree");
        fflush(stdout);
        return reward;
    }
};