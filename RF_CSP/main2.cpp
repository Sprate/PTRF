
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
    int multi_test=0;
    io->recv_data(&multi_test, sizeof(int));
    vector<double> multi_all_time_csp;
    vector<uint64_t> multi_all_bytes;
    for(int multi_times=0;multi_times<multi_test;++multi_times)
    {
        //auto all_tree_start=clock_start();
        cout<<"test "<<multi_times<<endl;
        SemiHonestGen<NetIO> *gen= nullptr;
        SemiHonestEva<NetIO> *eva= nullptr;
        setup_semi_honest(io, party,gen,eva);
        cout<<"initial byets"<<io->counter<<endl;
        auto ot_start=clock_start();
        eva->ot->setup_recv();
        cout<<"base ot bytes "<<io->counter<<endl;
        cout<<"base ot time "<<time_from(ot_start)<<endl;
        io->flush();
        int forest_size=0;
        io->recv_data(&forest_size, sizeof(int));
        vector<long>all_time_csp;
        vector<uint64_t>all_tree_bytes;
        for(int tree_begin=0;tree_begin<forest_size;++tree_begin)
        {
            auto one_tree_begin=io->counter;
            auto start_csp=clock();
            auto start=clock_start();
            csp.recvCiper(io);
            io->flush();
            //csp.Step2();
            eva_circuit_encryptd(eva,csp);
            double t= time_from(start);
            //all_time_csp.push_back(t);
            cout<<"bob all time "<<t<<endl;
            cout<<"bob all bytes "<<eva->io->counter<<endl;
            io->flush();
            csp.step4(io);
            csp.clearCsp();
            auto one_tree_bytes=io->counter-one_tree_begin;
            all_tree_bytes.push_back(one_tree_bytes);
            cout<<"csp send all bytes "<<one_tree_bytes<<endl;
            auto end_csp=clock();
            cout<<"csp time is "<<(end_csp-start_csp)*1000/CLOCKS_PER_SEC <<" ms "<<endl;
            all_time_csp.push_back((end_csp-start_csp)*1000000/CLOCKS_PER_SEC);
            initial_eva(eva);
        }
        double time=0;
        for(auto a:all_time_csp)
        {
            time+=a/1000.0;
        }
        multi_all_time_csp.push_back(time);
        uint64_t forest_bytes=0;
        for(auto b:all_tree_bytes)
        {
            forest_bytes+=b;
        }
        multi_all_bytes.push_back(forest_bytes);
        delete eva;
        delete gen;
    }

    //cout<<"csp all time is "<<
    int index=0;
    double time=0;
    for(auto t:multi_all_time_csp)
    {
        time+=t;
        cout<<index++<<" "<<t<<ends;
    }
    cout<<endl;
    cout<<"multi test csp time is "<<time<<" average "<<time/multi_test<<" ms "<<endl;
    uint64_t  bytes=0;
    for(auto b:multi_all_bytes )
    {
        bytes+=b;
        cout<<b<<ends;
    }
    cout<<endl<<"multi test csp bytes is expected bast ot "<<bytes<<" average "<<bytes*1.0/(multi_test*1024)<<"KB"<<endl;
    delete io;
    return 0;
}