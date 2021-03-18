//
// Created by bonjour on 2020/2/16.
//

#include "emp-sh2pc/emp-sh2pc.h"
#include <NTL/ZZ.h>
#include "emp-sh2pc/bob_eva.h"
#include "CSP.h"

using namespace emp;
using namespace std;
using namespace NTL;

int main() {
    int port, party;
    party=BOB;
    port=12345;
    NetIO * io = new NetIO(party==ALICE ? nullptr : "192.168.1.120", port);
    CSP csp;
    csp.recvKey(io);

    SemiHonestGen<NetIO> *gen= nullptr;
    SemiHonestEva<NetIO> *eva= nullptr;
    setup_semi_honest(io, party,gen,eva);
    cout<<"initial byets"<<io->counter<<endl;
    auto ot_start=clock_start();
    eva->ot->setup_recv();
    cout<<"base ot bytes "<<io->counter<<endl;
    cout<<"base ot time "<<time_from(ot_start)<<endl;
    io->flush();
    int multi_test=0;
    io->recv_data(&multi_test, sizeof(int));
    vector<double>multi_test_time;
    vector<double >multi_test_bytes;
    for(int i=0;i<multi_test;++i)
    {
        auto start=clock();
        while (1)
        {
            int begin=0;
            cout<<456465<<endl;
            io->recv_data(&begin, sizeof(int));
            cout<<999<<endl;
            if(begin==12345) csp.train_step_2(io,eva);
            else break;
        }
        auto end=clock();
        cout<<" all time csp is "<<(end-start)*1000/CLOCKS_PER_SEC<<" ms "<<endl;
        multi_test_time.push_back((end-start)*1000.0/CLOCKS_PER_SEC);
        cout<<" all bytes is "<<io->counter*1.0/1024<<" KB "<<endl;
        multi_test_bytes.push_back(io->counter*1.0/1024);
    }
    for(int i=0;i<multi_test;i++)
    {
        cout<<i<<" "<<multi_test_time[i]<<ends;
    }
    cout<<endl;
    cout<<"csp bytes "<<(multi_test_bytes[multi_test-1]-multi_test_bytes[multi_test-2])/1024.0<<endl;
    delete gen;
    delete eva;
    delete io;
    return 0;
}