//
// Created by qhh on 2019/12/26.
//

#ifndef RF_PRIVATERANDOMFOREST_H
#define RF_PRIVATERANDOMFOREST_H

#include <iostream>
#include <vector>
#include <string>
#include "PrivateNode.h"
#include <map>
#include "PrivateTree.h"

using namespace std;
//class Tree;
extern const int PrivateNumOfTest;
extern const int PrivateNumOfTypes;

class PrivateRandomForest {
private:
    vector<pair<int,vector<NTL::ZZ>>>all_samples;
    vector<pair<int,vector<NTL::ZZ>>>test_samples;
    vector<vector<int>>result;
    vector<PrivateTree *>forest;
public:
    bool readTrainSample(string file,SP&sp);
    bool buildTree(int num,SP&sp,SemiHonestGen<NetIO> *gen);
    ~PrivateRandomForest();
    bool readTestSample(string test,SP&sp);
    //void predict(vector<double>&error);
    PrivateRandomForest();
    void build(SP&sp,SemiHonestGen<NetIO> *gen);
    void writeForest(string file);
    void writeForestModel(string file);
    void setAllSamples(vector<pair<int,vector<NTL::ZZ>>>&all){all_samples=all;}
    void setTestSamples(vector<pair<int,vector<NTL::ZZ>>>&test){test_samples=test;}
};

#endif //RF_PRIVATERANDOMFOREST_H
