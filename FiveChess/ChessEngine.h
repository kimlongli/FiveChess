#pragma once

#include <string>
#include <vector>
using namespace std;

namespace ChessEngine {

    enum Role { HUMAN = 1, COMPUTOR = 2, EMPTY = 0 };

    //位置结构体，行是x，列是y
    struct Position {
        int x;
        int y;
        int score;
        Position() {}
        Position(int x, int y) {
            this->x = x;
            this->y = y;
            score = 0;
        }
        Position(int x, int y, int score) {
            this->x = x;
            this->y = y;
            this->score = score;
        }
        bool operator <(const Position &pos) const {
            if (score != pos.score) {
                return score > pos.score;
            }
            if (x != pos.x) {
                return x < pos.x;
            }
            else {
                return y < pos.y;
            }
        }
    };


    //在开始之前，一些初始化工作
    void beforeStart();

    //判断是否是某一方赢了
    int isSomeOneWin();

    //悔棋
    string takeBack();

    //清除之前的记录，重新开局
    string reset(int role);

    //重新设置层数
    void setLevel(int level);

    //取得刚才电脑下得那一步棋子的位置
    Position getLastPosition();

    //人类下棋，返回棋盘，传给界面
    string nextStep(int x, int y);

    //获取棋谱
    vector<Position> getChessManual();


};
//namespace end
