//
// Created by Administrator on 2019/11/26.
//

#ifndef RF_SP_H
#define RF_SP_H

#include "Paillier/Paillier.h"
#include "RandomForest/RandomForest.h"
#include "emp-sh2pc/emp-sh2pc.h"
using namespace std;
using namespace emp;
class SP :public Paillier {
public:
    NTL::ZZ PDO(const NTL::ZZ &cipertext );
    explicit SP(Paillier &p);
    SP();
    ~SP();
    void step1(const vector<NTL::ZZ>& enc_features);
    void step1_2(const vector<NTL::ZZ>&enc_features) ;
    void step3(const NTL::ZZ& enc_tree_r,bool encrypted_threshold);
    void step3_2(NTL::ZZ enc_tree_r);

    vector<vector<int>> genPermutationMatrix();
    vector<vector<int>> genPermutationMatrix_2();
    vector<NTL::ZZ> dataTransform(const vector<NTL::ZZ> &enc_features);
    //void step1(const map<int, pair<int, long>>  &decision_nodes,const vector<NTL::ZZ>& enc_features);

    void sendCiper(NetIO * io);
    void recvCmpResult(NetIO * io);
    void sendValueAndPathCost(NetIO *io);

    vector<pair<NTL::ZZ,NTL::ZZ>> getCiperPair(){ return ciper_pair; }
    vector<NTL::ZZ> getBlindC(){return blind_C;}
    vector<NTL::ZZ>getBlindR(){ return blind_r;}
    void setDecisionNodes(map<int,pair<int,int64_t >>&_decision_nodes){this->decision_nodes=_decision_nodes;this->m=_decision_nodes.size();}
    void setEncDecisionNodes(map<int,pair<int,NTL::ZZ >>&_enc_decision_nodes){this->enc_decision_nodes=_enc_decision_nodes;this->m=_enc_decision_nodes.size();}
    void setLeafNodes(map<int,vector<pair<int,int>>> &_leaf_nodes,map<int,int>&_leaf_values)
    {this->leaf_nodes=_leaf_nodes;this->leaf_values=_leaf_values;this->leaf_num=_leaf_nodes.size();}
    map<int,pair<int,int64_t >>getDecisionNodes(){ return decision_nodes;}
    vector<long>getThreshold(){ return threshold;}
    vector<NTL::ZZ>getEncThreshold(){return enc_threshold;}
    vector<bool>getBlindZ(){return blind_Z;}

    int m=0;
    void genEncLeafValues();
    void genEncRi();
    void genEncZi();
    void genPathCostRAndValueR();
    vector<double>offline_time;
    void setDecD(vector<bool>&_dec_d){this->dec_d = _dec_d;}
    void genEncDecD();
    void clearAllData();

private:
    NTL::ZZ lambda1;
    vector<pair<NTL::ZZ,NTL::ZZ>>ciper_pair;
    vector<NTL::ZZ> blind_C;
    vector<NTL::ZZ>blind_r;
    vector<vector<int>>matrix;
    vector<long>threshold;
    vector<NTL::ZZ>enc_threshold;
    map<int,pair<int,int64_t >>decision_nodes;
    map<int,pair<int,NTL::ZZ >>enc_decision_nodes;
    map<int,int>leaf_values;
    map<int,NTL::ZZ>enc_leaf_values;
    map<int,vector<pair<int,int>>>leaf_nodes;
    vector<bool>blind_Z;
    vector<NTL::ZZ>blind_cmp_result;
    vector<NTL::ZZ>enc_ri;
    vector<NTL::ZZ>enc_Z;
    vector<NTL::ZZ>cmp_result;
    map<int,vector<NTL::ZZ>>edge_cost;
    map<int,NTL::ZZ>path_cost;
    vector<long>path_cost_r;
    vector<long>leaf_value_r;
    vector<pair<NTL::ZZ, NTL::ZZ>> path_cost_and_value;
    vector<pair<NTL::ZZ,NTL::ZZ>> pack_path_cost_and_value;
    vector<pair<NTL::ZZ, NTL::ZZ>> path_cost_and_value_C1;
    int leaf_num=0;
    pair<int,int>package_information;
    vector<bool>dec_d;
    vector<NTL::ZZ>enc_dec_d;

};


#endif //RF_SP_H
