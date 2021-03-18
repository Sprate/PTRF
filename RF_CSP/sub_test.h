//
// Created by bonjour on 2021/1/11.
//

#ifndef RF_CSP_SUB_TEST_H
#define RF_CSP_SUB_TEST_H
#include "emp-sh2pc/emp-sh2pc.h"
#include <NTL/ZZ.h>
#include "emp-sh2pc/bob_eva.h"
#include "CSP.h"

using namespace emp;
using namespace std;
using namespace NTL;

void test_SecCH {
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
    //io->recv_data(&package_num, sizeof(int));
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
    int table_size;
    cout<<"start point is"<<m128i_to_string(eva->gc->start_point)<<endl;
    eva->io->recv_data(&size, sizeof(int));
    block *b_label=new block[size*64];
    block *y_label=new block[size*64];
    bool *di=new bool[size];

    eva->io->recv_block(b_label,size*64);
    eva->io->recv_block(y_label,size*64);
    eva->io->recv_data(&table_size,sizeof(int));

    cout<<"table size is "<<table_size<<endl;
    block *tables=new block[table_size];
    eva->io->recv_block(tables,table_size);
    eva->io->flush();
    FileIO *iowrite=new FileIO("eva_circuits.txt",0);
    iowrite->send_block(tables,table_size);
    iowrite->flush();
    delete iowrite;

    FileIO *ioread =new FileIO("eva_circuits.txt",1);
    eva->gc->set_evafile_io(ioread);

    vector<pair<NTL::ZZ,NTL::ZZ>> ciper_pair;
    for(int i=0;i<ciper_pair_size;++i)
    {
        ZZ c=ZZFromBytes(C+i*ciper_bytes,ciper_bytes);
        ZZ c1=ZZFromBytes(C1+i*ciper_bytes,ciper_bytes);
        ciper_pair.push_back(make_pair(c,c1));
    }
    //
    /*
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
     */

    //

    vector<NTL::ZZ> blind_plain;
    for(vector<pair<NTL::ZZ,NTL::ZZ>>::size_type i=0;i<ciper_pair.size();++i)
    {
        NTL::ZZ tmp;
        tmp=csp.PDT(ciper_pair[i].first,ciper_pair[i].second);
        blind_plain.push_back(tmp);
    }


    Integer b[size];
    Integer y[size];
    Integer a[size];
    Integer c[size];
    Bit result[size];
    block a_label[size*64];
    bool a_bool[size*64];
    NTL::ZZ ring =NTL::power_ZZ(2,64);
    for(int i=0;i<size;++i)
    {
        Integer::bool_data(a_bool+i*64,64,NTL::to_long(blind_plain[i]%ring));
    }
    cout<<"before ot time "<<time_from(start)<<endl;
    cout<<"before ot bytes "<<eva->io->counter-bytes<<endl;
    eva->ot->recv_impl(a_label,a_bool,size*64);
    cout<<"ot bytes "<<eva->io->counter-bytes<<endl;
    cout<<"ot time"<<time_from(start)<<endl;

    for(int i=0;i<size;++i)
    {
        b[i]=Integer (64,b_label+i*64);
        y[i]=Integer (64,y_label+i*64);
        a[i]=Integer (64,a_label+i*64);
        c[i]=a[i]-b[i];
        result[i]=c[i].geq(y[i]);
    }
    cout<<"eva time "<<time_from(start)<<endl;


    bool *result_bool=new bool[size];
    for(int i=0;i<size;++i)
    {
        result_bool[i]=getLSB(result[i].bit);
        cout<<"the result is::"<<result_bool[i]<<endl;
    }
    cout<<"reveal time "<<time_from(start)<<endl;
    cout<<"eva send all bytes "<<eva->io->counter-bytes<<endl;
    auto tt=clock_start();
    unsigned char*p=new unsigned char[size*ciper_bytes];
    for(int i=0;i<size;++i)
    {
        ZZ tmp=csp.encrypt(ZZ(result_bool[i]));
        BytesFromZZ(p+i*ciper_bytes,tmp,ciper_bytes);
    }
    eva->io->send_data(p,size*ciper_bytes);
    eva->io->flush();
    cout<<(long)time_from(csp_start_high)/1000<<endl;
    cout<<" cmp rersult transform time is "<<time_from(tt)<<endl;
    auto csp_end=clock();
    cout<<"CSP all bytes is"<<(eva->io->counter-begin_bytes)/1000<<endl;
    cout<<"CSP all time is "<<(csp_end-csp_start)*1000/CLOCKS_PER_SEC<<endl;
    delete []p;
    delete[] b_label;
    delete[] y_label;
    delete[] di;
    delete[]tables;
    delete ioread;
    delete[] result_bool;
    delete []C;
    delete []C1;

    delete io;

}
void test_SecCE()
{
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
    int table_size;
    cout<<"start point is"<<m128i_to_string(eva->gc->start_point)<<endl;
    eva->io->recv_data(&size, sizeof(int));
    block *b_label=new block[size*64];
    //block *y_label=new block[size*64];
    bool *di=new bool[size];

    eva->io->recv_block(b_label,size*64);
    //eva->io->recv_block(y_label,size*64);
    eva->io->recv_data(&table_size,sizeof(int));

    cout<<"table size is "<<table_size<<endl;
    block *tables=new block[table_size];
    eva->io->recv_block(tables,table_size);
    eva->io->flush();
    FileIO *iowrite=new FileIO("eva_circuits.txt",0);
    iowrite->send_block(tables,table_size);
    iowrite->flush();
    delete iowrite;

    FileIO *ioread =new FileIO("eva_circuits.txt",1);
    eva->gc->set_evafile_io(ioread);

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
    Integer y[size];
    Integer a[size];
    Integer c[size];
    Bit result[size];
    block a_label[size*64];
    bool a_bool[size*64];
    NTL::ZZ ring =NTL::power_ZZ(2,64);
    for(int i=0;i<size;++i)
    {
        long long z_mod=NTL::to_long(blind_plain[i]%ring)+(uint64_t)pow(2,63);
        Integer::bool_data(a_bool+i*64,64,z_mod);
    }
    vector<bool>z_l_1;
    for(int i=0;i<size;++i)
    {
        bool zz=(blind_plain[i]>>64)%2;
        z_l_1.push_back(zz);
    }
    cout<<"before ot time "<<time_from(start)<<endl;
    cout<<"before ot bytes "<<eva->io->counter-bytes<<endl;
    eva->ot->recv_impl(a_label,a_bool,size*64);
    cout<<"ot bytes "<<eva->io->counter-bytes<<endl;
    cout<<"ot time"<<time_from(start)<<endl;

    for(int i=0;i<size;++i)
    {
        b[i]=Integer (64,b_label+i*64);
        //y[i]=Integer (64,y_label+i*64);
        a[i]=Integer (64,a_label+i*64);
        result[i]=a[i].geq(b[i]);
    }
    cout<<"eva time "<<time_from(start)<<endl;


    bool *result_bool=new bool[size];
    for(int i=0;i<size;++i)
    {
        result_bool[i]=getLSB(result[i].bit);
        cout<<"the result is::"<<result_bool[i]<<endl;
    }
    cout<<"reveal time "<<time_from(start)<<endl;
    cout<<"eva send all bytes "<<eva->io->counter-bytes<<endl;
    auto tt=clock_start();
    unsigned char*p=new unsigned char[size*ciper_bytes];
    for(int i=0;i<size;++i)
    {
        ZZ tmp=csp.encrypt(ZZ(result_bool[i]^z_l_1[i]));
        BytesFromZZ(p+i*ciper_bytes,tmp,ciper_bytes);
    }
    eva->io->send_data(p,size*ciper_bytes);
    eva->io->flush();
    cout<<(long)time_from(csp_start_high)/1000<<endl;
    cout<<" cmp rersult transform time is "<<time_from(tt)<<endl;
    auto csp_end=clock();
    cout<<"CSP all bytes is"<<(eva->io->counter-begin_bytes)/1000<<endl;
    cout<<"CSP all time is "<<(csp_end-csp_start)*1000/CLOCKS_PER_SEC<<endl;
    delete []p;
    delete[] b_label;
    //delete[] y_label;
    delete[] di;
    delete[]tables;
    delete ioread;
    delete[] result_bool;
    delete []C;
    delete []C1;

    delete io;

}
#endif //RF_CSP_SUB_TEST_H
