//
// Created by bonjour on 2021/1/9.
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
    auto csp_start=clock();

    long ciper_pair_size=0;
    long ciper_bytes=csp.length_n*2;
    int package_num=0;
    auto begin_bytes=io->counter;
    io->recv_data(&ciper_pair_size, sizeof(long));
    io->recv_data(&package_num, sizeof(int));
    auto csp_start_high=clock_start();
    //num_of_pakage=ciper_pair_size;
    //io->recv_data(&last_package_num, sizeof(int));
    unsigned char *C=new unsigned char[ciper_pair_size*ciper_bytes];
    unsigned char *C1=new unsigned char[ciper_pair_size*ciper_bytes];
    io->recv_data(C,ciper_pair_size*ciper_bytes);
    io->recv_data(C1,ciper_pair_size*ciper_bytes);

    uint64_t bytes=eva->io->counter;
    int size;
    auto start=clock_start();
    long table_size;
    int index_bit=10;
    int feature_bit=64;
    cout<<"start point is"<<m128i_to_string(eva->gc->start_point)<<endl;
    eva->io->recv_data(&size, sizeof(int));
    block *b_label=new block[size*feature_bit];
    block *index_i_label=new block[size*index_bit];

    bool *send_di=new bool[size*index_bit];

    eva->io->recv_block(b_label,size*feature_bit);
    eva->io->recv_block(index_i_label,size*index_bit);

    // eva->io->recv_block(y_label,size*64);
    eva->io->recv_data(&table_size,sizeof(long));

    cout<<"table size is "<<table_size<<endl;
    block *tables=new block[table_size];
    eva->io->recv_block(tables,table_size);
    eva->io->flush();
    FileIO *iowrite=new FileIO("eva_circuits_sort.txt",0);
    iowrite->send_block(tables,table_size);
    iowrite->flush();
    delete iowrite;

    FileIO *ioread =new FileIO("eva_circuits_sort.txt",1);
    eva->gc->set_evafile_io(ioread);
    eva->io->recv_data(send_di,size*index_bit );
    eva->di=send_di;

    vector<pair<NTL::ZZ,NTL::ZZ>> ciper_pair;
    for(int i=0;i<ciper_pair_size;++i)
    {
        ZZ c=ZZFromBytes(C+i*ciper_bytes,ciper_bytes);
        ZZ c1=ZZFromBytes(C1+i*ciper_bytes,ciper_bytes);
        ciper_pair.push_back(make_pair(c,c1));
    }
    //

    vector<NTL::ZZ> blind_plain;
    NTL::ZZ expand_factor=NTL::PowerMod(NTL::ZZ(2),106,csp.modulus);
    for(vector<pair<NTL::ZZ,NTL::ZZ>>::size_type i=0;i<ciper_pair.size()-1;++i)
    {
        NTL::ZZ tmp;
        tmp=csp.PDT(ciper_pair[i].first,ciper_pair[i].second);
        for(int j=0;j<9;++j)
        {
            NTL::ZZ t=tmp%expand_factor;
            tmp=tmp/expand_factor;
            blind_plain.push_back(t);
            //cout<<tmp<<endl;
        }
    }
    NTL::ZZ tmp=csp.PDT(ciper_pair[ciper_pair.size()-1].first,ciper_pair[ciper_pair.size()-1].second);
    if(package_num==0)
    {
        for(int j=0;j<9;++j)
        {
            NTL::ZZ t=tmp%expand_factor;
            tmp=tmp/expand_factor;
            blind_plain.push_back(t);
        }
    }
    if(package_num!=0)
    {
        for(int j=0;j<package_num;++j)
        {
            NTL::ZZ t=tmp%expand_factor;
            tmp=tmp/expand_factor;
            blind_plain.push_back(t);
            //cout<<tmp<<endl;
        }

    }
    //
    /*
    vector<NTL::ZZ> blind_plain;
    for(vector<pair<NTL::ZZ,NTL::ZZ>>::size_type i=0;i<ciper_pair.size();++i)
    {
        NTL::ZZ tmp;
        tmp=csp.PDT(ciper_pair[i].first,ciper_pair[i].second);
        blind_plain.push_back(tmp);
    }
    */


    Integer b[size];
    Integer a[size];
    Integer c[size];
    Integer index_i[size];
    block a_label[size*feature_bit];
    bool a_bool[size*feature_bit];
    NTL::ZZ ring=NTL::power_ZZ(2,64);
    for(int i=0;i<size;++i)
    {
        Integer::bool_data(a_bool+i*feature_bit,feature_bit,NTL::to_long(blind_plain[i]%ring));
    }
    cout<<"before ot time "<<time_from(start)<<endl;
    //eva->ot->setup_recv();
    //cout<<"base ot time "<<time_from(start)<<endl;
    cout<<"before ot bytes "<<eva->io->counter-bytes<<endl;
    eva->ot->recv_impl(a_label,a_bool,size*feature_bit);
    cout<<"ot bytes "<<eva->io->counter-bytes<<endl;
    cout<<"ot time"<<time_from(start)<<endl;

    for(int i=0;i<size;++i)
    {
        b[i]=Integer (feature_bit,b_label+i*feature_bit);
        a[i]=Integer (feature_bit,a_label+i*feature_bit);
        index_i[i]=Integer(index_bit,index_i_label+i*index_bit);
        c[i]=a[i]-b[i];
        //c[i].resize(64,1);
    }
    sort(c,size,index_i);
    cout<<"sort"<<endl;
    cout<<"eva time "<<time_from(start)<<endl;


    for(int i=0;i<size;++i) cout<<send_di[i]<<ends;
    cout<<endl;
    int64_t *sort_result =new int64_t[size];
    for(int i=0;i<size;++i)
    {

        sort_result[i]=index_i[i].reveal<long >(BOB);

    }
    cout<<"sort resut is"<<endl;
    for(int i=0;i<size;++i) cout<<sort_result[i]<<ends;
    cout<<endl;
    cout<<"reveal time "<<time_from(start)<<endl;
    cout<<"eva send all bytes "<<eva->io->counter-bytes<<endl;
    auto tt=clock_start();

    eva->io->send_data(sort_result,size* sizeof(int64_t));
    eva->io->flush();
    eva->di=nullptr;

    cout<<" sort result transform time is "<<time_from(tt)<<endl;
    auto csp_end=clock();
    cout<<(long)time_from(csp_start_high)<<endl;
    cout<<"CSP all bytes is"<<(eva->io->counter-begin_bytes)<<endl;
    cout<<"CSP all time is "<<(csp_end-csp_start)*1000*1000/CLOCKS_PER_SEC<<endl;
    delete[] b_label;
    delete[] send_di;
    delete[]tables;
    delete ioread;
    delete[] sort_result;

    delete []C;
    delete []C1;

    delete io;

}