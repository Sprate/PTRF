//
// Created by qhh on 2019/12/26.
//

#ifndef RF_PRIVATETREE_H
#define RF_PRIVATETREE_H
#include <map>
#include "PrivateNode.h"
#include <NTL/ZZ.h>
#include "emp-sh2pc/alice_gen.h"


class PrivateTree {
private:
    PrivateNode *root;
public:
    PrivateTree();
    bool readPrivateSample(vector<pair<int,vector<NTL::ZZ>>> &all_samples,int Coe);
    bool buildPrivateTree(SP&sp,SemiHonestGen<NetIO> *gen);
    pair<PrivateNode*,PrivateNode*>selectBestSplite(PrivateNode *current,SP&sp,SemiHonestGen<NetIO> *gen);
    pair<PrivateNode*,PrivateNode*>selectBestSplite_2(PrivateNode *current,SP&sp,SemiHonestGen<NetIO> *gen);
    vector<int>randomSelect();
    pair<int,double > predict(vector<NTL::ZZ>features);
    ~PrivateTree();
    map<int,pair<int,NTL::ZZ>>getDecisionNodes();
    pair<map<int, vector<pair<int,int>>>,map<int,int>> getLeafNodes();
};


#endif //RF_PRIVATETREE_H
