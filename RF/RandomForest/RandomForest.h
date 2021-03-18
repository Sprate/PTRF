//
// Created by Administrator on 2019/11/22.
//

#ifndef RF_RANDOMFOREST_H
#define RF_RANDOMFOREST_H

#include <iostream>
#include <vector>
#include <string>
#include "Node.h"
#include <map>
#include "Tree.h"

using namespace std;
//class Tree;
extern const int NumOfTest;
extern const int NumOfTypes;

class RandomForest {
private:
    vector<pair<int,vector<double>>>all_samples;
    vector<pair<int,vector<double>>>test_samples;
    vector<vector<int>>result;
    vector<Tree *>forest;
public:
    bool readTrainSample(string file);
    bool buildTree(int num);
    ~RandomForest();
    bool readTestSample(string test);
    void predict(vector<double>&error);
    RandomForest();
    void build();
    void writeForest(string file);
    void writeForestModel(string file);
    void setAllSamples(vector<pair<int,vector<double>>>&all){all_samples=all;}
    void setTestSamples(vector<pair<int,vector<double>>>&test){test_samples=test;}
};


#endif //RF_RANDOMFOREST_H
