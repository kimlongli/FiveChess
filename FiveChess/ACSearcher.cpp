#include "stdafx.h"
#include "ACSearcher.h"

#include <cassert>


ACSearcher::ACSearcher()
    :maxState(0)
{
    //初始化根节点
    AddState(-1, 'a');
    nodes[0].fail = -1;

}


ACSearcher::~ACSearcher()
{
}


void ACSearcher::LoadPattern(const vector<string>& paterns) {
    this->paterns = paterns;
}

void ACSearcher::BuildGotoTable() {
    assert(nodes.size());

    unsigned int i, j;
    for (i = 0; i < paterns.size(); i++) {
        //从根节点开始
        int currentIndex = 0;
        for (j = 0; j < paterns[i].size(); j++) {
            if (nodes[currentIndex].sons.find(paterns[i][j]) == nodes[currentIndex].sons.end()) {
                nodes[currentIndex].sons[paterns[i][j]] = ++maxState;

                //生成新节点
                AddState(currentIndex, paterns[i][j]);
                currentIndex = maxState;
            }
            else {
                currentIndex = nodes[currentIndex].sons[paterns[i][j]];
            }
        }

        nodes[currentIndex].output.push_back(i);
    }
}

void ACSearcher::BuildFailTable() {
    assert(nodes.size());

    //中间节点收集器
    vector<int> midNodesIndex;

    //给第一层的节点设置fail为0，并把第二层节点加入到midState里
    ACNode root = nodes[0];

    map<char, int>::iterator iter1, iter2;
    for (iter1 = root.sons.begin(); iter1 != root.sons.end(); iter1++) {
        nodes[iter1->second].fail = 0;
        ACNode &currentNode = nodes[iter1->second];

        //收集第三层节点
        for (iter2 = currentNode.sons.begin(); iter2 != currentNode.sons.end(); iter2++) {
            midNodesIndex.push_back(iter2->second);
        }
    }

    //广度优先遍历
    while (midNodesIndex.size()) {
        vector<int> newMidNodesIndex;

        unsigned int i;
        for (i = 0; i < midNodesIndex.size(); i++) {
            ACNode &currentNode = nodes[midNodesIndex[i]];

            //以下循环为寻找当前节点的fail值
            int currentFail = nodes[currentNode.parent].fail;
            while (true) {
                ACNode &currentFailNode = nodes[currentFail];

                if (currentFailNode.sons.find(currentNode.ch) != currentFailNode.sons.end()) {
                    //成功找到该节点的fail值
                    currentNode.fail = currentFailNode.sons.find(currentNode.ch)->second;

                    //后缀包含
                    if (nodes[currentNode.fail].output.size()) {
                        currentNode.output.insert(currentNode.output.end(), nodes[currentNode.fail].output.begin(), nodes[currentNode.fail].output.end());
                    }

                    break;
                }
                else {
                    currentFail = currentFailNode.fail;
                }

                //如果是根节点
                if (currentFail == -1) {
                    currentNode.fail = 0;
                    break;
                }
            }

            //收集下一层节点
            for (iter1 = currentNode.sons.begin(); iter1 != currentNode.sons.end(); iter1++) {
                //收集下一层节点
                newMidNodesIndex.push_back(iter1->second);
            }
        }
        midNodesIndex = newMidNodesIndex;
    }
}

vector<int> ACSearcher::ACSearch(const string& text) {
    vector<int> result;

    //初始化为根节点
    int currentIndex = 0;

    unsigned int i;
    map<char, int>::iterator tmpIter;
    for (i = 0; i < text.size();) {
        //顺着trie树查找
        if ((tmpIter = nodes[currentIndex].sons.find(text[i])) != nodes[currentIndex].sons.end()) {
            currentIndex = tmpIter->second;
            i++;
        }
        else {
            //失配的情况
            while (nodes[currentIndex].fail != -1 && nodes[currentIndex].sons.find(text[i]) == nodes[currentIndex].sons.end()) {
                currentIndex = nodes[currentIndex].fail;
            }

            //如果没有成功找到合适的fail
            if (nodes[currentIndex].sons.find(text[i]) == nodes[currentIndex].sons.end()) {
                i++;
            }
        }

        if (nodes[currentIndex].output.size()) {
            result.insert(result.end(), nodes[currentIndex].output.begin(), nodes[currentIndex].output.end());
        }

    }

    return result;
}

void ACSearcher::AddState(int parent, char ch) {
    nodes.push_back(ACNode(parent, ch));
    assert(nodes.size() - 1 == maxState);
}
