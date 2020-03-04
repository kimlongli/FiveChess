#pragma once

#include <string>
#include <vector>
#include <map>
using namespace std;

//trie树节点
struct ACNode {
    ACNode(int p, char c)
        :parent(p),
        ch(c),
        fail(-1)
    {
    }

    char ch;
    map<char, int> sons;
    int fail;
    vector<int> output;
    int parent;
};

//AC算法类
class ACSearcher
{
public:
    ACSearcher();
    ~ACSearcher();

    void LoadPattern(const vector<string>& paterns);
    void BuildGotoTable();
    void BuildFailTable();
    vector<int> ACSearch(const string& text);           //返回匹配到的模式的索引

private:
    int maxState;                                       //最大状态数
    vector<ACNode> nodes;                               //trie树
    vector<string> paterns;                             //需要匹配的模式

    void AddState(int parent, char ch);                                    //初始化新状态
};

