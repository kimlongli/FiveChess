#include "stdafx.h"
#include "ChessEngine.h"

#include <iostream>
#include <cstdlib>
#include <set>
#include <vector>
#include <cmath>
#include <stack>
using namespace std;

namespace ChessEngine {

#define BOARD_WIDTH 15
#define UNKNOWN_SCORE (10000001)
#define HASH_ITEM_INDEX_MASK (0xffff)
#define MAX_SCORE (10000000)
#define MIN_SCORE (-10000000)
int DEPTH = 7;

enum Role { HUMAN = 1, COMPUTOR = 2, EMPTY = 0 };


//特征结构体，存储诸如"++OOO++"这样的特征
struct Mode {
    char str[10];    //特征字符串
    int score;
    Mode(const char str[10], int score) {
        strncpy_s(this->str, str, 10);
        this->score = score;
    }

    bool operator <(const Mode &mode) const {
        return strcmp(str, mode.str) < 0 ? true : false;
    }
};

//保存棋局的哈希表
struct HashItem {
    long long checksum;
    int depth;
    int score;
    enum Flag { ALPHA = 0, BETA = 1, EXACT = 2, EMPTY = 3 } flag;
};

long long boardZobristValue[2][BOARD_WIDTH][BOARD_WIDTH];
long long currentZobristValue;
HashItem hashItems[HASH_ITEM_INDEX_MASK + 1];
char board[BOARD_WIDTH][BOARD_WIDTH];
int winner;     //胜出者

stack<Position> moves;
int scores[2][72];  //保存棋局分数（2个角色72行，包括横竖撇捺）
int allScore[2];    //局面总评分（2个角色）

//记录计算结果在哈希表中
void recordHashItem(int depth, int score, HashItem::Flag flag) {
    int index = (int)(currentZobristValue & HASH_ITEM_INDEX_MASK);
    HashItem *phashItem = &hashItems[index];

    if (phashItem->flag != HashItem::EMPTY && phashItem->depth > depth) {
        return;
    }

    phashItem->checksum = currentZobristValue;
    phashItem->score = score;
    phashItem->flag = flag;
    phashItem->depth = depth;
}


//在哈希表中取得计算好的局面的分数
int getHashItemScore(int depth, int alpha, int beta) {
    int index = (int)(currentZobristValue & HASH_ITEM_INDEX_MASK);
    HashItem *phashItem = &hashItems[index];

    if (phashItem->flag == HashItem::EMPTY)
        return UNKNOWN_SCORE;

    if (phashItem->checksum == currentZobristValue) {
        if (phashItem->depth >= depth) {
            if (phashItem->flag == HashItem::EXACT) {
                return phashItem->score;
            }
            if (phashItem->flag == HashItem::ALPHA && phashItem->score <= alpha) {
                return alpha;
            }
            if (phashItem->flag == HashItem::BETA && phashItem->score >= beta) {
                return beta;
            }
        }
    }

    return UNKNOWN_SCORE;
}

//生成64位随机数
long long random64() {
    return (long long)rand() | ((long long)rand() << 15) | ((long long)rand() << 30) | ((long long)rand() << 45) | ((long long)rand() << 60);
}

//生成zobrist键值
void randomBoardZobristValue() {
    int i, j, k;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < BOARD_WIDTH; j++) {
            for (k = 0; k < BOARD_WIDTH; k++) {
                boardZobristValue[i][j][k] = random64();
            }
        }
    }
}

//初始化初始局面的zobrist值
void initCurrentZobristValue() {
    currentZobristValue = random64();
}

//存储特征的set
set<Mode> modes;
//存储搜索结果，即下一步棋子的位置
Position searchResult;


//根据位置评分，其中board是当前棋盘，p是位置，role是评分角色，比如role是Human则是相对人类评分，比如role是computer则是对于电脑评分
int evaluatePoint(char board[BOARD_WIDTH][BOARD_WIDTH], Position p) {
    int result, i, j;
    int role;

    result = 0;
    role = HUMAN;


    string lines[4];
    string lines1[4];
    for (i = max(0, p.x - 4); i < min(BOARD_WIDTH, p.x + 5); i++) {
        if (i != p.x) {
            lines[0].push_back(board[i][p.y] == role ? '1' : board[i][p.y] == 0 ? '0' : '2');
            lines1[0].push_back(board[i][p.y] == role ? '2' : board[i][p.y] == 0 ? '0' : '1');
        }
        else {
            lines[0].push_back('1');
            lines1[0].push_back('1');
        }
    }
    for (i = max(0, p.y - 4); i < min(BOARD_WIDTH, p.y + 5); i++) {
        if (i != p.y) {
            lines[1].push_back(board[p.x][i] == role ? '1' : board[p.x][i] == 0 ? '0' : '2');
            lines1[1].push_back(board[p.x][i] == role ? '2' : board[p.x][i] == 0 ? '0' : '1');
        }
        else {
            lines[1].push_back('1');
            lines1[1].push_back('1');
        }
    }
    for (i = p.x - min(min(p.x, p.y), 4), j = p.y - min(min(p.x, p.y), 4); i < min(BOARD_WIDTH, p.x + 5) && j < min(BOARD_WIDTH, p.y + 5); i++, j++) {
        if (i != p.x) {
            lines[2].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
            lines1[2].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');
        }
        else {
            lines[2].push_back('1');
            lines1[2].push_back('1');
        }
    }
    for (i = p.x + min(min(p.y, BOARD_WIDTH - 1 - p.x), 4), j = p.y - min(min(p.y, BOARD_WIDTH - 1 - p.x), 4); i >= max(0, p.x - 4) && j < min(BOARD_WIDTH, p.y + 5); i--, j++) {
        if (i != p.x) {
            lines[3].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
            lines1[3].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');
        }
        else {
            lines[3].push_back('1');
            lines1[3].push_back('1');
        }
    }

    set<Mode>::iterator iter;
    for (i = 0; i < 4; i++) {
        for (j = 0; j + 5 <= (int)lines[i].length();) {
            iter = modes.find(Mode(lines[i].substr(j, 6).c_str(), 0));
            if (iter != modes.end()) {
                result += (*iter).score;
                j += 5;
                continue;
            }

            iter = modes.find(Mode(lines[i].substr(j, 5).c_str(), 0));
            if (iter != modes.end()) {
                result += (*iter).score;
                j += 5;
                continue;
            }

            j++;
        }
        for (j = 0; j + 5 <= (int)lines1[i].length();) {
            iter = modes.find(Mode(lines1[i].substr(j, 6).c_str(), 0));
            if (iter != modes.end()) {
                result += (*iter).score;
                j += 5;
                continue;
            }

            iter = modes.find(Mode(lines1[i].substr(j, 5).c_str(), 0));
            if (iter != modes.end()) {
                result += (*iter).score;
                j += 5;
                continue;
            }

            j++;
        }
    }

    return result;
}


//局面评估函数，给一个局面评分
int evaluate(char board[BOARD_WIDTH][BOARD_WIDTH], Role role) {

    if (role == COMPUTOR) {
        return allScore[1];
    }
    else if (role == HUMAN) {
        return allScore[0];
    }

    cout << "error" << endl;

    return 0;
}


void updateScore(char board[BOARD_WIDTH][BOARD_WIDTH], Position p) {

    string lines[4];
    string lines1[4];
    int i, j;
    int role = HUMAN;

    //竖
    for (i = 0; i < BOARD_WIDTH; i++) {

        lines[0].push_back(board[i][p.y] == role ? '1' : board[i][p.y] == 0 ? '0' : '2');
        lines1[0].push_back(board[i][p.y] == role ? '2' : board[i][p.y] == 0 ? '0' : '1');


    }
    //横
    for (i = 0; i < BOARD_WIDTH; i++) {

        lines[1].push_back(board[p.x][i] == role ? '1' : board[p.x][i] == 0 ? '0' : '2');
        lines1[1].push_back(board[p.x][i] == role ? '2' : board[p.x][i] == 0 ? '0' : '1');

    }
    //反斜杠
    for (i = p.x - min(p.x, p.y), j = p.y - min(p.x, p.y); i < BOARD_WIDTH && j < BOARD_WIDTH; i++, j++) {

        lines[2].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
        lines1[2].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');

    }
    //斜杠
    for (i = p.x + min(p.y, BOARD_WIDTH - 1 - p.x), j = p.y - min(p.y, BOARD_WIDTH - 1 - p.x); i >= 0 && j < BOARD_WIDTH; i--, j++) {

        lines[3].push_back(board[i][j] == role ? '1' : board[i][j] == 0 ? '0' : '2');
        lines1[3].push_back(board[i][j] == role ? '2' : board[i][j] == 0 ? '0' : '1');

    }

    int lineScore[4];
    int line1Score[4];
    memset(lineScore, 0, sizeof(lineScore));
    memset(line1Score, 0, sizeof(line1Score));

    set<Mode>::iterator iter;
    for (i = 0; i < 4; i++) {
        for (j = 0; j + 5 <= (int)lines[i].length();) {
            iter = modes.find(Mode(lines[i].substr(j, 6).c_str(), 0));
            if (iter != modes.end()) {
                lineScore[i] += (*iter).score;
                j += 5;
                continue;
            }

            iter = modes.find(Mode(lines[i].substr(j, 5).c_str(), 0));
            if (iter != modes.end()) {
                lineScore[i] += (*iter).score;
                j += 5;
                continue;
            }

            j++;
        }

        for (j = 0; j + 5 <= (int)lines1[i].length();) {
            iter = modes.find(Mode(lines1[i].substr(j, 6).c_str(), 0));
            if (iter != modes.end()) {
                line1Score[i] += (*iter).score;
                j += 5;
                continue;
            }

            iter = modes.find(Mode(lines1[i].substr(j, 5).c_str(), 0));
            if (iter != modes.end()) {
                line1Score[i] += (*iter).score;
                j += 5;
                continue;
            }

            j++;
        }
    }


    int a = p.y;
    int b = BOARD_WIDTH + p.x;
    int c = 2 * BOARD_WIDTH + (p.y - p.x + 10);
    int d = 2 * BOARD_WIDTH + 21 + (p.x + p.y - 4);
    //减去以前的记录
    for (i = 0; i < 2; i++) {
        allScore[i] -= scores[i][a];
        allScore[i] -= scores[i][b];
    }



    //scores顺序 竖、横、\、/
    scores[0][a] = lineScore[0];
    scores[1][a] = line1Score[0];
    scores[0][b] = lineScore[1];
    scores[1][b] = line1Score[1];


    //加上新的记录
    for (i = 0; i < 2; i++) {
        allScore[i] += scores[i][a];
        allScore[i] += scores[i][b];
    }



    if (p.y - p.x >= -10 && p.y - p.x <= 10) {

        for (i = 0; i < 2; i++)
            allScore[i] -= scores[i][c];


        scores[0][c] = lineScore[2];
        scores[1][c] = line1Score[2];

        for (i = 0; i < 2; i++)
            allScore[i] += scores[i][c];

    }

    if (p.x + p.y >= 4 && p.x + p.y <= 24) {

        for (i = 0; i < 2; i++)
            allScore[i] -= scores[i][d];

        scores[0][d] = lineScore[3];
        scores[1][d] = line1Score[3];

        for (i = 0; i < 2; i++)
            allScore[i] += scores[i][d];
    }
}


//生成下一步可以走的位置
set<Position> createPossiblePosition(char board[BOARD_WIDTH][BOARD_WIDTH]) {
    int i, j;

    set<Position> possiblePossitions;
    for (i = 0; i < BOARD_WIDTH; i++) {
        for (j = 0; j < BOARD_WIDTH; j++) {
            if (board[i][j] != 0) {
                //left
                if (i > 0 && board[i - 1][j] == 0) {
                    possiblePossitions.insert(Position(i - 1, j, evaluatePoint(board, Position(i - 1, j))));
                }
                //right
                if (i < BOARD_WIDTH - 1 && board[i + 1][j] == 0) {
                    possiblePossitions.insert(Position(i + 1, j, evaluatePoint(board, Position(i + 1, j))));
                }
                //up
                if (j > 0 && board[i][j - 1] == 0) {
                    possiblePossitions.insert(Position(i, j - 1, evaluatePoint(board, Position(i, j - 1))));
                }
                //down
                if (j < BOARD_WIDTH - 1 && board[i][j + 1] == 0) {
                    possiblePossitions.insert(Position(i, j + 1, evaluatePoint(board, Position(i, j + 1))));
                }
                //left up
                if (i > 0 && j > 0 && board[i - 1][j - 1] == 0) {
                    possiblePossitions.insert(Position(i - 1, j - 1, evaluatePoint(board, Position(i - 1, j - 1))));
                }
                //left down
                if (i < BOARD_WIDTH - 1 && j > 0 && board[i + 1][j - 1] == 0) {
                    possiblePossitions.insert(Position(i + 1, j - 1, evaluatePoint(board, Position(i + 1, j - 1))));
                }
                //right up
                if (i > 0 && j < BOARD_WIDTH - 1 && board[i - 1][j + 1] == 0) {
                    possiblePossitions.insert(Position(i - 1, j + 1, evaluatePoint(board, Position(i - 1, j + 1))));
                }
                //right down
                if (i < BOARD_WIDTH - 1 && j < BOARD_WIDTH - 1 && board[i + 1][j + 1] == 0) {
                    possiblePossitions.insert(Position(i + 1, j + 1, evaluatePoint(board, Position(i + 1, j + 1))));
                }

            }
        }
    }

    return possiblePossitions;
}

//alpha-beta剪枝
int abSearch(char board[BOARD_WIDTH][BOARD_WIDTH], int depth, int alpha, int beta, Role currentSearchRole) {
    HashItem::Flag flag = HashItem::ALPHA;
    int score = getHashItemScore(depth, alpha, beta);
    if (score != UNKNOWN_SCORE && depth != DEPTH) {
        return score;
    }

    int score1 = evaluate(board, currentSearchRole);
    int score2 = evaluate(board, currentSearchRole == HUMAN ? COMPUTOR : HUMAN);


    if (score1 >= 50000) {
        return MAX_SCORE - 1000 - (DEPTH - depth);
    }
    if (score2 >= 50000) {
        return MIN_SCORE + 1000 + (DEPTH - depth);
    }


    if (depth == 0) {
        recordHashItem(depth, score1 - score2, HashItem::EXACT);
        return score1 - score2;
    }

    set<Position> possiblePossitions = createPossiblePosition(board);


    int count = 0;
    while (!possiblePossitions.empty()) {
        Position p = *possiblePossitions.begin();

        possiblePossitions.erase(possiblePossitions.begin());
        //放置棋子
        board[p.x][p.y] = currentSearchRole;
        currentZobristValue ^= boardZobristValue[currentSearchRole - 1][p.x][p.y];
        updateScore(board, p);


        int val = -abSearch(board, depth - 1, -beta, -alpha, currentSearchRole == HUMAN ? COMPUTOR : HUMAN);
        if (depth == DEPTH)
            cout << "score(" << p.x << "," << p.y << "):" << val << endl;

        //取消放置
        board[p.x][p.y] = 0;
        currentZobristValue ^= boardZobristValue[currentSearchRole - 1][p.x][p.y];
        updateScore(board, p);

        if (val >= beta) {
            recordHashItem(depth, beta, HashItem::BETA);
            return beta;
        }
        if (val > alpha) {
            flag = HashItem::EXACT;
            alpha = val;
            if (depth == DEPTH) {
                searchResult = p;
            }
        }

        count++;
        if (count >= 9) {
            break;
        }
    }

    recordHashItem(depth, alpha, flag);
    return alpha;

}


//获得下一步的走法
Position getAGoodMove(char board[BOARD_WIDTH][BOARD_WIDTH]) {
    int score = abSearch(board, DEPTH, MIN_SCORE, MAX_SCORE, COMPUTOR);
    if (score >= MAX_SCORE - 1000 - 1) {
        winner = COMPUTOR;
    }
    else if (score <= MIN_SCORE + 1000 + 1) {
        winner = HUMAN;
    }
    return searchResult;
}



//初始化函数，插入特征和分值
void init() {
    modes.insert(Mode("11111", 50000));
    modes.insert(Mode("011111", 50000));
    modes.insert(Mode("111110", 50000));
    modes.insert(Mode("011110", 4320));
    modes.insert(Mode("011100", 720));
    modes.insert(Mode("001110", 720));
    modes.insert(Mode("011010", 720));
    modes.insert(Mode("010110", 720));
    modes.insert(Mode("11110", 720));
    modes.insert(Mode("01111", 720));
    modes.insert(Mode("11011", 720));
    modes.insert(Mode("10111", 720));
    modes.insert(Mode("11101", 720));
    modes.insert(Mode("001100", 120));
    modes.insert(Mode("001010", 120));
    modes.insert(Mode("010100", 120));
    modes.insert(Mode("000100", 20));
    modes.insert(Mode("001000", 20));


    randomBoardZobristValue();
    currentZobristValue = random64();
}

//输出棋盘
void printBoard(char board[BOARD_WIDTH][BOARD_WIDTH]) {
    int i, j;
    for (i = 0; i < BOARD_WIDTH; i++) {
        for (j = 0; j < BOARD_WIDTH; j++) {
            cout << (int)board[i][j] << " ";
        }
        cout << endl;
    }
}



////以下是对外接口的实现

//在开始之前，一些初始化工作
void beforeStart() {
    memset(board, EMPTY, BOARD_WIDTH * BOARD_WIDTH * sizeof(char));
    memset(scores, 0, sizeof(scores));
    init();
}

//判断是否是某一方赢了
int isSomeOneWin() {
    if (winner == HUMAN)
        return 0;
    if (winner == COMPUTOR)
        return 1;

    return -1;
}

//悔棋
string takeBack() {
    if (moves.size() < 2) {
        cout << "can not take back" << endl;

        string resultStr;
        int i, j;
        for (i = 0; i < BOARD_WIDTH; i++) {
            for (j = 0; j < BOARD_WIDTH; j++) {
                resultStr.push_back(board[i][j] + 48);
            }
        }

        printBoard(board);

        return resultStr;
    }


    Position previousPosition = moves.top();
    moves.pop();
    currentZobristValue ^= boardZobristValue[COMPUTOR - 1][previousPosition.x][previousPosition.y];
    board[previousPosition.x][previousPosition.y] = EMPTY;
    updateScore(board, previousPosition);


    previousPosition = moves.top();
    moves.pop();
    currentZobristValue ^= boardZobristValue[HUMAN - 1][previousPosition.x][previousPosition.y];
    board[previousPosition.x][previousPosition.y] = EMPTY;
    updateScore(board, previousPosition);

    string resultStr;
    int i, j;
    for (i = 0; i < BOARD_WIDTH; i++) {
        for (j = 0; j < BOARD_WIDTH; j++) {
            resultStr.push_back(board[i][j] + 48);
        }
    }

    printBoard(board);

    winner = -1;

    return resultStr;

}

//清除之前的记录，重新开局
string reset(int role) {
    char chs[15 * 15 + 1];
    memset(chs, '0', 15 * 15);
    memset(scores, 0, sizeof(scores));
    memset(allScore, 0, sizeof(allScore));

    winner = -1;
    //初始化局面总分数为0

    while (!moves.empty()) {
        moves.pop();
    }

    int i;
    for (i = 0; i < HASH_ITEM_INDEX_MASK + 1; i++) {
        hashItems[i].flag = HashItem::EMPTY;
    }


    //用户先走
    if (role == 0) {
        memset(board, EMPTY, BOARD_WIDTH * BOARD_WIDTH * sizeof(char));
    }
    //电脑先走
    else if (role == 1) {
        memset(board, EMPTY, BOARD_WIDTH * BOARD_WIDTH * sizeof(char));
        currentZobristValue ^= boardZobristValue[COMPUTOR - 1][7][7];
        board[7][7] = COMPUTOR;
        updateScore(board, Position(7, 7));

        moves.push(Position(7, 7));
        searchResult = Position(7, 7);

        //第一步默认走7，7的位置
        chs[7 + 7 * 15] = '2';
        return chs;
    }

    winner = -1;

    return string(chs);
}

//重新设置层数
void setLevel(int level) {
    DEPTH = level;
}

//取得刚才电脑下得那一步棋子的位置
Position getLastPosition() {
    return searchResult;
}

//人类下棋，返回棋盘，传给界面
string nextStep(int x, int y) {

    moves.push(Position(x, y));

    board[x][y] = HUMAN;
    currentZobristValue ^= boardZobristValue[HUMAN - 1][x][y];
    updateScore(board, Position(x, y));
    //printBoard(board);

    Position result = getAGoodMove(board);
    board[result.x][result.y] = COMPUTOR;
    currentZobristValue ^= boardZobristValue[COMPUTOR - 1][result.x][result.y];
    updateScore(board, result);
    //printBoard(board);

    //若双方还未决出胜负，则把棋子位置加入到历史记录中
    if(winner == -1)
        moves.push(Position(result.x, result.y));

    string resultStr;
    int i, j;
    for (i = 0; i < BOARD_WIDTH; i++) {
        for (j = 0; j < BOARD_WIDTH; j++) {
            resultStr.push_back(board[i][j] + 48);
        }
    }

    printBoard(board);

    return resultStr;
}

//获取棋谱
vector<Position> getChessManual() {
    vector<Position> result;

    while (!moves.empty()) {
        result.insert(result.begin(), moves.top());
        moves.pop();
    }

    return result;
}


}; //namespace end
