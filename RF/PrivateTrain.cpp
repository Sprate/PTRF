//
// Created by bonjour on 2020/2/16.
//

#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <ctime>
#include "Paillier/Paillier.h"
#include "SP.h"
#include "emp-sh2pc/emp-sh2pc.h"
#include "emp-sh2pc/alice_gen.h"
#include "PrivateRandomForest/PrivateRandomForest.h"
#include <ctime>

#include <NTL/ZZ.h>

#include "util.h"


using namespace std;
using namespace NTL;
int main() {

    Paillier auth;
    auth.KeyGen();
    SP sp=SP(auth);

    int port, party;
    party=ALICE;
    port=12345;
    NetIO * io = new NetIO(party==ALICE ? nullptr : "127.0.0.1", port);
    send_key(auth,io);
    cout<<"send key bytes: "<<io->counter<<endl;

    SemiHonestGen<NetIO> *gen= nullptr;
    SemiHonestEva<NetIO>* eva= nullptr;
    setup_semi_honest(io, party,gen,eva);
    auto ot_start=clock_start();
    gen->ot->setup_send();
    io->flush();
    cout<<"base ot time "<<time_from(ot_start)<<endl;
    cout<<"base ot send bytes "<<gen->io->counter<<endl;//just once

    clock_t start,end;

    int test_time=2;
    io->send_data(&test_time, sizeof(int));
    vector<double>multi_test_time;
    vector<double >multi_test_bytes;
    for(int i=0;i<test_time;++i)
    {
        PrivateRandomForest forest;
        forest.readTrainSample("/home/qhh/CLionProjects/code/RF/Touch/data/data_80_train.csv",sp);
        auto start_sp=clock_start();
        start=clock();
        //forest.readTestSample("test_wdbc.data",sp);
        forest.buildTree(50,sp,gen);
        end=clock();
        int train_end=1000;
        io->send_data(&train_end, sizeof(int));
        cout<<" RF private train run time is "<<(end-start)*1000/CLOCKS_PER_SEC<<" ms"<<endl;
        cout<<" RF private train run time all is "<<time_from(start_sp)/1000.0<<" ms "<<endl;
        multi_test_time.push_back(time_from(start_sp)/1000.0);
        multi_test_bytes.push_back(gen->io->counter*1.0/1024);
        cout<<" sp all bytes "<<gen->io->counter<<" B "<<gen->io->counter*1.0/1024<<" KB " <<endl;
        start=clock();
        //vector<double> error;
        //forest.predict(error);
        forest.writeForest("/home/qhh/CLionProjects/code/RF/Touch/private_tree/tree_80_50.txt");
        forest.writeForestModel("/home/qhh/CLionProjects/code/RF/Touch/private_tree/tree_model_80_50.txt");
        end=clock();
        cout<<" RF private classsify run time is "<<(end-start)*1000/CLOCKS_PER_SEC<<" ms"<<endl;
    }
    double min_time=10000000000000000000.0;
    for(int i=0;i<test_time;++i)
    {
        cout<<i<< " "<<multi_test_time[i]<<ends;
        min_time=multi_test_time[i]>min_time ?min_time :multi_test_time[i];
    }
    cout<<endl<<"min time is "<<min_time<<endl;
    cout<<"sp bytes is"<<(multi_test_bytes[test_time-1]-multi_test_bytes[test_time-2])/1024.0<<" MB"<<endl;
    return 0;
}