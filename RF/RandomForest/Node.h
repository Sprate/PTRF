//
// Created by Administrator on 2019/11/20.
//

#ifndef RF_NODE_H
#define RF_NODE_H

#include <stdio.h>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <NTL/ZZ.h>

using namespace std;
const int NumOfTypes=2;
const int NumOfFeatures=57;
const int MaxDepth=10;
const int NumOfThreads=8;
const int MaxFeatures=6;
const int NumOfTest=100;
const int NumOfTrees=50;
struct Node {
    int attribute;//当前节点是哪个特征
    double value;//节点分裂值
    NTL::ZZ private_value;//加密的节点分裂值
    Node *left;//左子节点
    Node *right;//右子节点
    int sample_num;//当前节点有多少个样本
    vector<int>num;//每个类型有多少样本
    int depth;//当前节点深度
    vector<pair<int,vector<double>>>samples;//<label,features>的数组
    vector<pair<int,vector<NTL::ZZ>>>private_samples;
    int index;
    Node();
    Node(const Node &other);
};


#endif //RF_NODE_H
