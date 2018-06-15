#pragma once

#include "ChessEngine.h"

#include <set>
#include <vector>

using namespace std;

struct HistoryItem {
    set<ChessEngine::Position> addedPositions;
    ChessEngine::Position removedPosition;
};

class PossiblePositionManager
{
public:
    PossiblePositionManager();
    ~PossiblePositionManager();
    void AddPossiblePositions(char board[15][15], const ChessEngine::Position& p);
    void Rollback();
    set<ChessEngine::Position> GetCurrentPossiblePositions();
    void RemoveAll();
    void SetEvaluateFunc(int(*evaluateFunc)(char board[15][15], ChessEngine::Position p));
private:
    set<ChessEngine::Position> currentPossiblePositions;
    vector<HistoryItem> history;
    vector<pair<int, int> > directions;
    int (*evaluateFunc)(char board[15][15], ChessEngine::Position p);
};

