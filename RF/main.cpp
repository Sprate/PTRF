#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include "RandomForest/RandomForest.h"
#include "RandomForest/Node.h"
#include "RandomForest/Tree.h"
#include <ctime>
#include "Paillier/Paillier.h"
#include "CSP.h"
#include "SP.h"
#include "test/test.h"
#include "emp-sh2pc/emp-sh2pc.h"
#include "emp-sh2pc/alice_gen.h"
#include <NTL/ZZ.h>
#include <RandomForest/ReadForest.h>
#include "util.h"
#include "RandomForest/ReadTestSample.h"

using namespace std;
using namespace NTL;
int main() {


    //tenFoldCross("wdbc.data",100);
   // transformUserBehaviour("data.csv");

/*
    clock_t start,end;
    start=clock();
    RandomForest forest;
    forest.readTrainSample("/home/qhh/CLionProjects/code/RF/UCL/spambase.data");
    forest.readTestSample("/home/qhh/CLionProjects/code/RF/UCL/spambase_test.csv");
    forest.buildTree(1);
    end=clock();
    cout<<" RF palin train run time is "<<(end-start)*1000/CLOCKS_PER_SEC<<" ms"<<endl;
    start=clock();
    vector<double> error;
    forest.predict(error);
    forest.writeForest("/home/qhh/CLionProjects/code/RF/UCL/tree1/tree_spambase.txt");
    forest.writeForestModel("/home/qhh/CLionProjects/code/RF/UCL/tree1/tree_spambase_model.txt");
    end=clock();
    cout<<" RF plain classsify run time is "<<(end-start)*1000/CLOCKS_PER_SEC<<" ms"<<endl;
*/
    vector<map<int, pair<int, int64_t >>>all_decision_nodes;
    vector<map<int,vector<pair<int,int>>>>all_leaf_nodes;
    vector<map<int,int>>all_leaf_values;
    readForest("/home/qhh/CLionProjects/code/RF/UCL/tree1/tree_spambase_model.txt",all_decision_nodes,all_leaf_nodes,all_leaf_values);
    Paillier auth;
    auth.KeyGen();
    SP sp=SP(auth);
    vector<pair<int,vector<int64_t >>>_features;
    vector<pair<int,vector<int64_t >>>features;
    readSample("/home/qhh/CLionProjects/code/RF/UCL/spambase_test.csv",_features);
    //vector<long> features={13170,2590,2370,20000,120000,1650,680,530,1460,9300,600,1620,840000};//3
    //vector<NTL::ZZ> enc_features;
    for(int i=0;i<100;++i)
    {
        features.push_back(_features[0]);
    }
    vector<pair<int,vector<NTL::ZZ>>>enc_features;
    //user 待验证特征
    cout<<"encrpt features begin.."<<endl;

    for(const auto &fea:features)
    {   vector<NTL::ZZ>tmp;
        for(const auto tmp1:fea.second)
        {
            NTL::ZZ enc_tmp1(tmp1);
            tmp.push_back(auth.encrypt(enc_tmp1));
            cout<<enc_tmp1<<endl;
        }
        enc_features.emplace_back(fea.first,tmp);
    }
    cout<<"encrypt features end ..."<<endl;

    int port, party;
    party=ALICE;
    port=12345;
    NetIO * io = new NetIO(party==ALICE ? nullptr : "192.168.1.121", port);
    send_key(auth,io);
    cout<<"send key bytes: "<<io->counter<<endl;
    int multi_test=features.size();
    cout<<multi_test<<endl;
    io->send_data(&multi_test, sizeof(int));
    vector<double>multi_all_online_time;
    vector<double>multi_all_offline_time;
    vector<uint64_t >multi_all_tree_bytes;
    for(const auto &enc_feature:enc_features)
    {
        auto all_tree_time=clock_start();
        SemiHonestGen<NetIO> *gen= nullptr;
        SemiHonestEva<NetIO>* eva= nullptr;
        setup_semi_honest(io, party,gen,eva);
        cout<<"initial bytes "<<io->counter<<endl;
        auto ot_start=clock_start();
        gen->ot->setup_send();
        io->flush();
        cout<<"base ot time "<<time_from(ot_start)<<endl;
        cout<<"base ot send bytes "<<gen->io->counter<<endl;//just once

        vector<int>all_result;
        vector<double>all_online_time;
        vector<double>all_offline_time;
        vector<uint64_t >all_tree_bytes;
        int forest_size=1;
        vector<NTL::ZZ> tree_r;
        for(auto i=0;i<forest_size;++i)
        {
            tree_r.push_back(NTL::RandomLen_ZZ(80));
        }
        io->send_data(&forest_size, sizeof(int));
        // begin all tree

        for(int tree_counts=0;tree_counts<forest_size;tree_counts++)
        {
            cout<<endl;
            cout<<endl;
            cout<<endl;
            cout<<"begin one tree "<<endl;
            auto sp_time=clock_start();
            auto one_tree_send_bytes_begin=io->counter;


            NTL::ZZ enc_tree_r=sp.encrypt(NTL::ZZ(tree_r[tree_counts]));

            map<int,int>leaf_values=all_leaf_values[tree_counts];
            map<int,vector<pair<int,int>>>leaf_nodes=all_leaf_nodes[tree_counts];
            map<int, pair<int, int64_t >>decision_nodes=all_decision_nodes[tree_counts];


            //CSP csp=CSP(auth);

            //begin SP

            sp.setDecisionNodes(decision_nodes);
            sp.setLeafNodes(leaf_nodes,leaf_values);
            cout<<"push back offline time set decision nodes and leaf nodes "<<time_from(sp_time)<<"us"<<endl;
            sp.offline_time.push_back(time_from(sp_time));

            sp.step1(enc_feature.second);

            sp.sendCiper(io);
            cout<<"send ciper bytes: "<<io->counter<<endl;
            io->flush();
            //csp.Step2(sp.getCiperPair());

            gen_circuits(ALICE,sp.m,gen,sp);

            auto cmp=clock_start();
            sp.recvCmpResult(gen->io);//包括了bob 加密转换输出结果的时间
            cout<<"recv cmp result time is "<<time_from(cmp)<<endl;

            cout<<"all cost bytes "<<io->counter<<endl;

            cout<<"sp step 1 and recv cmp result all time is "<<time_from(sp_time)<<endl;
            io->flush();

            sp.step3(enc_tree_r,false);

            sp.sendValueAndPathCost(io);
            io->flush();
            cout<<" send value and path cost bytes "<<io->counter<<endl;

            auto *vj_r_char=new unsigned char[256];
            io->recv_data(vj_r_char, 10);
            NTL::ZZ vj_r=ZZFromBytes(vj_r_char,256);
            delete[] vj_r_char;
            long vj=NTL::to_long(vj_r-tree_r[tree_counts]);
            cout<<"result value is "<<vj<<endl;
            all_result.push_back(vj);
            double one_tree_time=time_from(sp_time);
            double offline_time=0;
            for(const auto t :sp.offline_time)
            {
                offline_time+=t;
            }
            cout<<"num of nodes and number of tree "<<sp.m<<" "<<sp.m+1<<endl;
            cout<<"one tree all time is(except base ot) "<<one_tree_time<<"us"<<endl;
            cout<<"one tree offline time is "<<offline_time<<"us"<<endl;
            all_offline_time.push_back(offline_time);
            all_online_time.push_back(one_tree_time-offline_time);
            cout<<"ont tree online time is "<<one_tree_time-offline_time<<"us"<<endl;
            cout<<"one tree send bytes(except base ot) "<<io->counter-one_tree_send_bytes_begin<<endl;
            all_tree_bytes.push_back(io->counter-one_tree_send_bytes_begin);
            sp.clearAllData();
            init_circuits(gen);
        }
        cout<<"all tree time is "<<time_from(all_tree_time)/1000<<"ms"<<endl;
        cout<<"real label "<<enc_feature.first<<" predict label "<<ends;
        for(const auto &result:all_result)
        {
            cout<<result<<ends;
        }
        cout<<endl;
        double all_time=0;
        for(const auto &time:all_offline_time)
        {
            all_time+=time;
        }
        multi_all_offline_time.push_back(all_time);
        cout<<"a forest offline time "<<all_time/1000<<"ms "<<"average "<<all_time/(forest_size*1.0)<<"us"<<endl;
        all_time=0;
        for(const auto &time:all_online_time)
        {
            all_time+=time;
        }
        multi_all_online_time.push_back(all_time);
        cout<<"a forest online time "<<all_time/1000<<"ms "<<"average "<<all_time/(forest_size*1.0)<<"us"<<endl;
        uint64_t all_bytes=0;
        for(const auto &bytes:all_tree_bytes)
        {
            all_bytes+=bytes;
        }
        multi_all_tree_bytes.push_back(all_bytes);
        cout<<"a forest all bytes sp is "<<all_bytes<<" average "<<all_bytes/forest_size<<endl;

    }

    int index=0;
    double multi_time=0;
    for(auto time:multi_all_offline_time)
    {
        multi_time+=time;
        cout<<index++<<" "<<time<<ends;
    }
    cout<<endl;
    cout<<" ten test all offline time is "<<multi_time/1000<< "ms average "<<multi_time/(multi_test*1000)<< "ms"<<endl;
    multi_time=0;
    index=0;
    double min_time=10000000;
    for(auto time:multi_all_online_time)
    {
        multi_time+=time;
        cout<<index++<<" "<<time<<ends;
        min_time=time>min_time ?min_time :time;
    }
    cout<<endl;
    cout<<" ten test all online time is "<<multi_time/1000<< "ms average "<<multi_time/(multi_test*1000)<< "ms"<<endl;
    cout<<"min _time "<<min_time<<endl;
    uint64_t multi_bytes=0;
    for(auto bytes:multi_all_tree_bytes)
    {
        multi_bytes+=bytes;
        cout<<bytes<<ends;
    }
    cout<<endl;
    cout<<" ten test all bytes time is "<<multi_bytes<< "bits   average "<<multi_bytes*1.0/(multi_test*1024)<<" KB "<<endl;
    delete io;


   /*int port, party;
    party=ALICE;
    port=12345;
    NetIO * io = new NetIO(party==ALICE ? nullptr : "127.0.0.1", port);
    auto start=clock_start();
    SemiHonestGen<NetIO> *gen= nullptr;
    SemiHonestEva<NetIO>* eva= nullptr;
    setup_semi_honest(io, party,gen,eva);
    cout<<"initial bytes "<<io->counter<<endl;
    auto ot_start=clock_start();
    gen->ot->setup_send();
    cout<<"base ot time "<<time_from(ot_start)<<endl;
    cout<<"base ot send bytes "<<gen->io->counter<<endl;
    gen_circuits(ALICE,64,gen);
    cout<<"all cost bytes "<<gen->io->counter<<endl;
    long long t= time_from(start);
    cout<<t<<endl;
    init_circuits(gen);
    cout<<m128i_to_string(gen->gc->start_point)<<endl;
    cout<<m128i_to_string(gen->gc->mitccrh.start_point)<<endl;
    delete io;*/
    return 0;
}