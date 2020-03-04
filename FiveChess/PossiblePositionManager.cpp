#include "stdafx.h"
#include "PossiblePositionManager.h"

#include <cassert>


bool IsInBoard(int x, int y) {
    if (x >= 0 && x < 15 && y >= 0 && y < 15)
        return true;
    return false;
}

PossiblePositionManager::PossiblePositionManager()
{
    directions.push_back(pair<int, int>(1, 1));
    directions.push_back(pair<int, int>(1, -1));
    directions.push_back(pair<int, int>(-1, 1));
    directions.push_back(pair<int, int>(-1, -1));
    directions.push_back(pair<int, int>(1, 0));
    directions.push_back(pair<int, int>(0, 1));
    directions.push_back(pair<int, int>(-1, 0));
    directions.push_back(pair<int, int>(0, -1));
}


PossiblePositionManager::~PossiblePositionManager()
{
}

void  PossiblePositionManager::AddPossiblePositions(char board[15][15], const ChessEngine::Position& p) {
    unsigned int i;
    set<ChessEngine::Position> addedPositions;

    for (i = 0; i < directions.size(); i++) {
        //判断范围
        if (!IsInBoard(p.x + directions[i].first, p.y + directions[i].second))
            continue;

        if (board[p.x + directions[i].first][p.y + directions[i].second] == ChessEngine::EMPTY) {
            ChessEngine::Position pos(p.x + directions[i].first, p.y + directions[i].second);
            pair<set<ChessEngine::Position>::iterator, bool> insertResult = currentPossiblePositions.insert(pos);

            //如果插入成功
            if(insertResult.second)
                addedPositions.insert(pos);
        }
    }

    HistoryItem hi;
    hi.addedPositions = addedPositions;

    if (currentPossiblePositions.find(p) != currentPossiblePositions.end()) {
        currentPossiblePositions.erase(p);
        hi.removedPosition = p;
    }
    else {
        hi.removedPosition.x = -1;
    }

    history.push_back(hi);
}

void PossiblePositionManager::Rollback() {
    if (currentPossiblePositions.empty())
        return;

    HistoryItem hi = history[history.size() - 1];
    history.pop_back();

    set<ChessEngine::Position>::iterator iter;

    //清除掉前一步加入的点
    for (iter = hi.addedPositions.begin(); iter != hi.addedPositions.end(); iter++) {
        currentPossiblePositions.erase(*iter);
    }

    //加入前一步删除的点
    if(hi.removedPosition.x != -1)
        currentPossiblePositions.insert(hi.removedPosition);
}

const set<ChessEngine::Position>& PossiblePositionManager::GetCurrentPossiblePositions() {
    return currentPossiblePositions;
}

void PossiblePositionManager::RemoveAll() {
    currentPossiblePositions.clear();
    history.clear();
}
