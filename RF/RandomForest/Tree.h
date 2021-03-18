//
// Created by Administrator on 2019/11/20.
//

#ifndef RF_TREE_H
#define RF_TREE_H

#include "Node.h"
#include <map>

class Tree{
private:
    Node *root;
public:
    Tree();
    bool readSample(vector<pair<int,vector<double>>> &all_samples,int Coe);
    bool buildTree();
    pair<Node*,Node*>selectBestSplite(Node *current);
    vector<int>randomSelect();
    pair<int,double>predict(vector<double>features);
    ~Tree();
    map<int,pair<int,double>>getDecisionNodes();
    pair<map<int, vector<pair<int,int>>>,map<int,int>> getLeafNodes();
};


#endif //RF_TREE_H
