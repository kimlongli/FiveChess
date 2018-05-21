#pragma once

#include "ChessEngine.h"

#include <string>
#include <vector>

using namespace std;

class ChessManual
{
public:
    ChessManual();
    ChessManual(const vector<ChessEngine::Position>& v, int whichFirst);
    ~ChessManual();
    bool ReadAndCheck(TCHAR *filename);         //读取并检查（格式、合法性等）
    string Next();                              //下一步
    string Previous();                          //上一步
    void SaveToFile(TCHAR *filename);           //保存到文件
    ChessEngine::Position GetLastPosition();    //获取当前下棋位置

private:
    int whichFirst;                         //黑子还是白子先走
    int currentIndex;                       //当前步数索引
    vector<ChessEngine::Position> moves;    //保存所有步数
    string board;                           //保存当前棋盘布局
    char currentRole;                       //当前下子角色，'1'或'2'

    void changeRole() {
        currentRole = currentRole == '1' ? '2' : '1';
    }
};

