#include "stdafx.h"
#include "ChessManual.h"

#include <fstream>

ChessManual::ChessManual()
    :whichFirst(0),
    currentIndex(-1)
{
    int i = 0;
    for (i = 0; i < 15 * 15; i++) {
        board.push_back('0');
    }
}

ChessManual::ChessManual(const vector<ChessEngine::Position>& v, int whichFirst)
    :whichFirst(whichFirst),
    currentIndex(-1),
    moves(v)
{
    int i = 0;
    for (i = 0; i < 15 * 15; i++) {
        board.push_back('0');
    }
}


bool ChessManual::ReadAndCheck(TCHAR *filename) {
    fstream fs(filename);

    //读入白字还是黑子先下
    fs >> whichFirst;
    if (whichFirst)
        currentRole = '2';
    else
        currentRole = '1';

    //初始化棋盘
    int i, j;
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 15; j++) {
            board[i * 15 + j] = '0';
        }
    }

    int x = -1, y = -1;
    while (fs >> x && fs >> y) {
        //判断x，y是否合法
        if (x >= 0 && x <= 14 && y >= 0 && y <= 14) {
            moves.push_back(ChessEngine::Position(x, y));
            if (board[x * 15 + y] == '0')
                board[x * 15 + y] = '1';
            else
                return false;
        }

        x = -1, y = -1;
    }

    //如果步数为0，返回false
    if (!moves.size())
        return false;

    //重置棋盘
    for (i = 0; i < 15; i++) {
        for (j = 0; j < 15; j++) {
            board[i * 15 + j] = '0';
        }
    }

    fs.close();
    return true;
}

string ChessManual::Next() {
    if (currentIndex == moves.size() - 1)
        return "";

    currentIndex++;
    ChessEngine::Position pos = moves[currentIndex];

    //在棋盘中下子
    board[pos.x * 15 + pos.y] = currentRole;

    //转换下子角色
    changeRole();

    return board;
}

string ChessManual::Previous() {
    if (currentIndex == -1)
        return "";

    ChessEngine::Position pos = moves[currentIndex];
    currentIndex--;

    //在棋盘中去除此位置的棋子
    board[pos.x * 15 + pos.y] = '0';

    //转换下子角色
    changeRole();

    return board;
}

void ChessManual::SaveToFile(TCHAR *filename) {
    fstream fs(filename, fstream::out);

    //写谁先下
    fs << whichFirst;
    fs << '\n';

    //写棋谱
    vector<ChessEngine::Position>::iterator iter;
    for (iter = moves.begin(); iter != moves.end(); iter++) {
        fs << (*iter).x << ' ' << (*iter).y << '\n';
    }

    fs.close();
}

ChessEngine::Position ChessManual::GetLastPosition() {
    if(currentIndex != -1)
        return moves[currentIndex];
    return ChessEngine::Position(-1, -1);
}

ChessManual::~ChessManual()
{
}
