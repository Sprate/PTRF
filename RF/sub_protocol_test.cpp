//
// Created by qhh on 2021/1/9.
//
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
    vector<long>a;

    vector<NTL::ZZ>enc_a;
    int test_num=64;
    for(int i=0;i<test_num;++i)
    {
        long tmp_a=NTL::RandomLen_long(63);

        NTL::ZZ enc_tmp_a=sp.encrypt(NTL::to_ZZ(tmp_a));

        a.push_back(tmp_a);

        enc_a.push_back(enc_tmp_a);

    }
    auto begin_time=clock();
    auto begin_time_high_precesion=clock_start();
    auto begin_bytes=gen->io->counter;
    vector<NTL::ZZ>r;
    vector<NTL::ZZ>enc_r;
    for(int i=0;i<test_num;++i)
    {
        NTL::ZZ tmp_r=NTL::RandomLen_ZZ(106);
        NTL::ZZ tmp_enc_r=sp.encrypt(tmp_r);
        r.push_back(tmp_r);
        enc_r.push_back(tmp_enc_r);
    }

    int size=test_num;
    auto start=clock_start();
    uint64_t bytes=gen->io->counter;
    int feature_bit=64;
    int index_bit=10;
    cout<<"before gen_circuits send bytes"<<bytes<<endl;
    Batcher batcher1,batcher2,batcher3;
    Integer*A=new Integer[size];
    Integer*B=new Integer[size];
    block *input_label0_alice=new block[size*feature_bit+size*index_bit];
    block *input_label0_bob=new block[size*feature_bit];

    bool *di=new bool[size*feature_bit];
    bool *send_di=new bool [size*index_bit];
    //PRG prg;
    gen->shared_prg.random_block(input_label0_bob,size*feature_bit);
    //prg.random_block(input_label0_bob,size*32);
    gen->shared_prg.random_block(input_label0_alice,size*feature_bit+size*index_bit);

    gen->input_label_alice=input_label0_alice;
    gen->input_label_bob=input_label0_bob;
    block *input_label1_bob=new block[size*feature_bit];
    for(int i=0;i<size *feature_bit;++i)
    {
        input_label1_bob[i]=xorBlocks(input_label0_bob[i],gen->gc->delta);
    }
    for(int i=0;i<size;++i)
    {
        batcher1.add<Integer>(feature_bit,0);
        batcher2.add<Integer>(feature_bit,0);
        batcher3.add<Integer>(index_bit,0);
    }
    batcher1.make_semi_honest(BOB);
    batcher2.make_semi_honest(ALICE);
    batcher3.make_semi_honest(ALICE);//给label
    gen->input_label_alice=gen->input_label_alice-size*feature_bit-size*index_bit;
    gen->input_label_bob=gen->input_label_bob-size*feature_bit;

    FileIO*iowrite =new FileIO("circuits_sort.txt",0);
    gen->gc->set_genfile_io(iowrite);

    for(int i=0;i<size;++i)
    {
        A[i]=batcher1.next<Integer>() - batcher2.next<Integer>();
        B[i]=batcher3.next<Integer>();
    }
    sort(A,size,B);
    for(int i=0;i<size;++i)
    {
        for(int j=0;j<feature_bit;++j)
        {
            di[i*feature_bit+j]=getLSB(A[i][j].bit);//低位到高位
        }
        for(int j=0;j<index_bit;++j)
        {
            send_di[i*index_bit+j]=getLSB(B[i][j].bit);
        }
    }
    cout<<"start point "<<m128i_to_string(gen->gc->start_point)<<endl;//hash nextindex 计数器
    cout<<"circuits gates table bytes "<<gen->gc->genfile->bytes_sent<<endl;
    long table_size=gen->gc->genfile->bytes_sent/16;
    iowrite->flush();
    delete iowrite;
    gen->di=di;//di 用不用到
    //cout<<gen->gc->io->counter<<endl;

    FileIO * ioread =new FileIO("circuits_sort.txt",1);
    block * tables= new block[table_size];

    //cout<<456<<endl;
    ioread->recv_data(tables,table_size* sizeof(block));
    ioread->flush();
    delete ioread;

    auto offline_time=clock();
    auto offline_time_high=time_from(begin_time_high_precesion);
    auto online_start=clock();
    auto online_start_high=clock_start();
    //online begin
    cout<<"online begin "<<endl;
    vector<NTL::ZZ>blind_C;
    vector<NTL::ZZ>blind_C1;
    //

    int num_of_package=enc_a.size()/9+1;
    int last_package_num=enc_a.size()%9;
    pair<int,int> package_information;
    NTL::ZZ expand_factor=NTL::PowerMod(NTL::ZZ(2),106,sp.modulus);
    for(int i=0;i<num_of_package-1;++i)
    {
        NTL::ZZ tmp=NTL::MulMod(enc_a[i*9+8],enc_r[i*9+8],sp.modulus*sp.modulus);
        for(int j=7;j>=0;--j)
        {
            NTL::ZZ t=NTL::MulMod(enc_a[i*9+j],enc_r[i*9+j],sp.modulus*sp.modulus);
            tmp=NTL::MulMod(NTL::PowerMod(tmp,expand_factor,sp.modulus*sp.modulus),t,sp.modulus*sp.modulus);
        }
        NTL::ZZ t2=sp.PDO(tmp);
        blind_C.push_back(tmp);
        blind_C1.push_back(t2);
    }
    if(last_package_num!=0){
        NTL::ZZ tmp=NTL::MulMod(enc_a[(num_of_package-1)*9+last_package_num-1],enc_r[(num_of_package-1)*9+last_package_num-1],sp.modulus*sp.modulus);
        for(int i=last_package_num-2;i>=0;--i)
        {
            NTL::ZZ t=NTL::MulMod(enc_a[(num_of_package-1)*9+i],enc_r[(num_of_package-1)*9+i],sp.modulus*sp.modulus);
            tmp=NTL::MulMod(NTL::PowerMod(tmp,expand_factor,sp.modulus*sp.modulus),t,sp.modulus*sp.modulus);
        }
        NTL::ZZ t2=sp.PDO(tmp);
        blind_C.push_back(tmp);
        blind_C1.push_back(t2);
        package_information.first=num_of_package;
        package_information.second=last_package_num;
    }
    if(last_package_num==0)
    {
        package_information.first=num_of_package-1;
        package_information.second=last_package_num;
    }

    /*
    for(int i=0;i<test_num;++i)
    {
        NTL::ZZ t=NTL::MulMod(enc_a[i],enc_r[i],sp.modulus*sp.modulus);
        NTL::ZZ t1=sp.PDO(t);
        blind_C.push_back(t);
        blind_C1.push_back(t1);
    }
     */

    auto enc_end=clock();
    auto *C=new unsigned char[blind_C.size()*256];
    auto *C1=new unsigned char[blind_C.size()*256];

    for(long i=0;i<blind_C.size();++i)
    {
        NTL::BytesFromZZ(C+i*256,blind_C[i],256);
        NTL::BytesFromZZ(C1+i*256,blind_C1[i],256);
    }
    long ciper_pair_size=blind_C.size();
    cout<<123<<endl;
    io->send_data(&ciper_pair_size, sizeof(long));
    io->send_data(&package_information.second, sizeof(int));
    io->send_data(C,ciper_pair_size*256);
    io->send_data(C1,ciper_pair_size*256);
    delete []C;
    delete []C1;

    Integer a_r[size];//bob y+r
    Integer b[size];//alice r
    Integer index_sample[size];
    NTL::ZZ ring=NTL::power_ZZ(2,64);
    for(int i=0;i<size;++i)
    {
        long long blind_mod_r=NTL::to_long(r[i]%ring);
        b[i]=Integer(feature_bit,blind_mod_r,ALICE);
    }
    for(long i=0;i<size;++i)
    {
        index_sample[i]=Integer(index_bit,i,ALICE);
    }
    double t2=time_from(start);
    cout<<"real input to garbled labels time "<<t2<<endl;

    block *b_lable=new block[size*feature_bit];
    for(int i=0;i<size;++i)
    {
        memcpy(b_lable+i*feature_bit,(block*)b[i].bits,feature_bit* sizeof(block));
        //gen->io->send_block((block*)b[i].bits,32);
    }
    block *index_i_label=new block[size *index_bit];
    for(int i=0;i<size;++i)
    {
        memcpy(index_i_label+i*index_bit,(block*)index_sample[i].bits,index_bit* sizeof(block));
    }
    gen->io->send_data(&size, sizeof(int));
    cout<<"size bytes :"<<gen->io->counter-bytes<<endl;

    gen->io->send_block(b_lable,size*feature_bit);
    gen->io->send_block(index_i_label,size*index_bit);

    cout<<"alice send her labels bytes "<<gen->io->counter-bytes<<endl;
    delete[] b_lable;
    delete[] index_i_label;
    cout<<"alice send input time "<<time_from(start)<<endl;

    gen->io->send_data(&table_size, sizeof(long));
    cout<<"table_size bytes: "<<gen->io->counter-bytes<<endl;
    gen->io->send_block(tables,table_size);
    gen->io->flush();
    cout<<"alice send tables bytes "<<gen->io->counter-bytes<<endl;
    long long t4=time_from(start);
    cout<<"alice send tables times "<<t4<<endl;
    gen->io->send_data(send_di,size*index_bit* sizeof(bool));
    //gen->ot->setup_send();
    // cout<<"bast ot "<<time_from(start)<<endl;
    gen->ot->send_impl(input_label0_bob,input_label1_bob,size*feature_bit);
    cout<<"ot bytes "<<gen->io->counter-bytes<<endl;
    cout<<" ot time "<<time_from(start)<<endl;
    //gen->io->send_data(di,size*feature_bit);
    for(int i=0;i<size*index_bit;++i) cout<<send_di[i]<<ends;
    cout<<endl;
    cout<<gen->io->counter-bytes<<endl;
    gen->io->flush();
    cout<<"all time"<<time_from(start)<<endl;

    delete[] A;
    delete[] input_label0_alice;
    delete[] input_label0_bob;
    delete[] input_label1_bob;
    delete[] tables;

    int64_t *sort_result=new int64_t [size];
    gen->io->recv_data(sort_result,size* sizeof(int64_t));
    gen->io->flush();
    for(int i=0;i<size;++i) cout<<sort_result[i]<<ends;

    auto all_end=clock();
    auto all_end_high=time_from(online_start_high);
    cout<<endl;
    for(int i=0;i<size;++i)
    {
        cout<<a[sort_result[i]]<<endl;
    }
    cout<<"offline time"<<(offline_time-begin_time)*1000*1000/CLOCKS_PER_SEC<<endl;
    cout<<"offline time high "<<(long)offline_time_high<<endl;
    cout<<"online time "<<(all_end-online_start)*1000000/CLOCKS_PER_SEC<<endl;
    cout<<"online time high "<<(long)all_end_high<<endl;
    double com_MB=(io->counter-begin_bytes)*1.0/(1024*1024);
    cout<<com_MB<<endl;
    cout<<(io->counter-begin_bytes)<<endl;
    return 0;
}

