//
// Created by qhh on 2019/12/26.
//

#ifndef RF_PRIVATENODE_H
#define RF_PRIVATENODE_H


#include <stdio.h>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <NTL/ZZ.h>

using namespace std;
const int PrivateNumOfTypes=2;
const int PrivateNumOfFeatures=28;
const int PrivateMaxDepth=10;
const int PrivateNumOfThreads=8;
const int PrivateMaxFeatures=5;
const int PrivateNumOfTest=100;
const int PrivateNumOfTrees=50;
struct PrivateNode {
    int attribute;//当前节点是哪个特征
    double value;//节点分裂值
    NTL::ZZ private_value;//加密的节点分裂值
    PrivateNode *left;//左子节点
    PrivateNode *right;//右子节点
    int sample_num;//当前节点有多少个样本
    vector<int>num;//每个类型有多少样本
    int depth;//当前节点深度
    vector<pair<int,vector<double>>>samples;//<label,features>的数组
    vector<pair<int,vector<NTL::ZZ>>>private_samples;
    int index;
    PrivateNode();
    PrivateNode(const PrivateNode &other);
};



#endif //RF_PRIVATENODE_H
