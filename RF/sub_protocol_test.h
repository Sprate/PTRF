//
// Created by qhh on 2021/1/11.
//

#ifndef RF_SUB_PROTOCOL_TEST_H
#define RF_SUB_PROTOCOL_TEST_H

#endif //RF_SUB_PROTOCOL_TEST_H
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
void test_SecCH {

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
    vector<long>b;
    vector<NTL::ZZ>enc_a;
    vector<NTL::ZZ>enc_b;
    for(int i=0;i<1000;++i)
    {
        long tmp_a=NTL::RandomLen_long(63);
        long tmp_b=NTL::RandomLen_long(63);
        NTL::ZZ enc_tmp_a=sp.encrypt(NTL::to_ZZ(tmp_a));
        NTL::ZZ enc_tmp_b=sp.encrypt(NTL::to_ZZ(tmp_b));
        a.push_back(tmp_a);
        b.push_back(tmp_b);
        enc_a.push_back(enc_tmp_a);
        enc_b.push_back(enc_tmp_b);
    }
    auto begin_time=clock();
    auto begin_time_high_precesion=clock_start();
    auto begin_bytes=gen->io->counter;
    vector<NTL::ZZ>r;
    vector<NTL::ZZ>enc_r;
    for(int i=0;i<1000;++i)
    {
        NTL::ZZ tmp_r=NTL::RandomLen_ZZ(106);
        NTL::ZZ tmp_enc_r=sp.encrypt(tmp_r);
        r.push_back(tmp_r);
        enc_r.push_back(tmp_enc_r);
    }
    int size=1000;
    int feature_bit=64;
    Batcher batcher1,batcher2,batcher3;
    Integer*A=new Integer[size];
    Bit *B=new Bit[size];
    block *input_label0_alice=new block[size*feature_bit*2];
    block *input_label0_bob=new block[size*feature_bit];
    bool *di=new bool[size];
    gen->shared_prg.random_block(input_label0_bob,size*feature_bit);
    gen->shared_prg.random_block(input_label0_alice,size*feature_bit*2);
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
        batcher3.add<Integer>(feature_bit,0);
    }
    batcher1.make_semi_honest(BOB);
    batcher2.make_semi_honest(ALICE);
    batcher3.make_semi_honest(ALICE);//给label
    gen->input_label_alice=gen->input_label_alice-size*feature_bit*2;
    gen->input_label_bob=gen->input_label_bob-size*feature_bit;

    FileIO*iowrite =new FileIO("circuits.txt",0);
    gen->gc->set_genfile_io(iowrite);
    for(int i=0;i<size;++i)
    {
        A[i]=batcher1.next<Integer>() - batcher2.next<Integer>();
        B[i]=A[i].geq(batcher3.next<Integer>());
        di[i]=getLSB(B[i].bit);
    }
    cout<<"start point "<<m128i_to_string(gen->gc->start_point)<<endl;//hash nextindex 计数器
    cout<<"circuits gates table bytes "<<gen->gc->genfile->bytes_sent<<endl;
    iowrite->flush();
    delete iowrite;
    gen->di=di;//di 用不用到
    //cout<<gen->gc->io->counter<<endl;
    vector<bool>dec_d;
    vector<NTL::ZZ>enc_dec_d;
    for(int i=0;i<size;++i)
    {
        dec_d.push_back(di[i]);
        enc_dec_d.push_back(sp.encrypt(NTL::ZZ(di[i])));
    }
    FileIO * ioread =new FileIO("circuits.txt",1);
    block * tables= new block[size*254];
    int table_size=size*254;
    ioread->recv_data(tables,table_size* sizeof(block));
    ioread->flush();
    delete ioread;
    delete[] A;
    delete[] B;
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
    for(int i=0;i<1000;++i)
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

    vector<long>blind_mod_r;
    NTL::ZZ ring=NTL::power_ZZ(2,64);
    for(auto tmp:r) blind_mod_r.push_back(NTL::to_long(tmp%ring));
    Integer R[size];//alice r
    Integer bb[size];//alice 阈值t
    for(int i=0;i<size;++i)
    {
        R[i]=Integer (feature_bit,blind_mod_r[i],ALICE);
    }
    blind_mod_r.clear();
    for(int i=0;i<size;++i)
    {
        bb[i]=Integer(feature_bit,b[i],ALICE);
    }
    block *r_lable=new block[size*feature_bit];
    block *b_lable=new block[size*feature_bit];
    for(int i=0;i<size;++i)
    {
        memcpy(r_lable+i*feature_bit,(block*)R[i].bits,feature_bit* sizeof(block));
        memcpy(b_lable+i*feature_bit,(block*)bb[i].bits,feature_bit* sizeof(block));
    }

    gen->io->send_data(&size, sizeof(int));
    cout<<"size bytes :"<<gen->io->counter-begin_bytes<<endl;

    gen->io->send_block(r_lable,size*feature_bit);
    gen->io->send_block(b_lable,size*feature_bit);
    cout<<"alice send her labels bytes "<<gen->io->counter-begin_bytes<<endl;
    delete[] r_lable;
    delete[] b_lable;

    gen->io->send_data(&table_size, sizeof(int));
    cout<<"table_size bytes: "<<gen->io->counter-begin_bytes<<endl;
    gen->io->send_block(tables,table_size);
    gen->io->flush();
    cout<<"alice send tables bytes "<<gen->io->counter-begin_bytes<<endl;

    gen->ot->send_impl(input_label0_bob,input_label1_bob,size*feature_bit);
    gen->io->flush();
    cout<<"ot bytes "<<gen->io->counter-begin_bytes<<endl;

    delete[] input_label0_alice;
    delete[] input_label0_bob;
    delete[] input_label1_bob;
    delete[] tables;

    vector<NTL::ZZ>blind_result;
    long ciper_bytes=NTL::NumBytes(sp.modulus)*2;
    auto*p=new unsigned char[size*ciper_bytes];
    io->recv_data(p,size*ciper_bytes);
    io->flush();
    for(int i=0;i<size;++i)
    {
        NTL::ZZ tmp=NTL::ZZFromBytes(p+i*ciper_bytes,ciper_bytes);
        blind_result.push_back(tmp);
    }
    delete []p;
    vector<int>cmp;
    vector<NTL::ZZ>cmp_enc;
    for(int i=0;i<size;++i)
    {
        NTL::ZZ tmp = NTL::MulMod(NTL::PowerMod(blind_result[i], NTL::ZZ(1 - 2 * dec_d[i]), sp.modulus *sp. modulus),
                                  enc_dec_d[i], sp.modulus * sp.modulus);
        cmp_enc.push_back(tmp);

    }
    auto all_end=clock();
    auto all_end_high=time_from(online_start_high);
    for(int i=0;i<size;++i)
    {
        cmp.push_back(NTL::to_int(auth.decrypt(cmp_enc[i])));
    }
    for(int i=0;i<size;++i)
    {
        cout<<a[i]<<" > "<<b[i]<<" "<<cmp[i]<<endl;
    }
    cout<<"offline time"<<(offline_time-begin_time)*1000/CLOCKS_PER_SEC<<endl;
    cout<<"offline time high "<<(long)offline_time_high/1000<<endl;
    cout<<"online time "<<(all_end-online_start)*1000/CLOCKS_PER_SEC<<endl;
    cout<<"online time high "<<(long)all_end_high/1000<<endl;
    cout<<(io->counter-begin_bytes)/1000<<endl;
    return 0;
}
void tset_SecCE()
{
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
    vector<long>b;
    vector<NTL::ZZ>enc_a;
    vector<NTL::ZZ>enc_b;
    for(int i=0;i<1000;++i)
    {
        long tmp_a=NTL::RandomLen_long(63);
        long tmp_b=NTL::RandomLen_long(63);
        NTL::ZZ enc_tmp_a=sp.encrypt(NTL::to_ZZ(tmp_a));
        NTL::ZZ enc_tmp_b=sp.encrypt(NTL::to_ZZ(tmp_b));
        a.push_back(tmp_a);
        b.push_back(tmp_b);
        enc_a.push_back(enc_tmp_a);
        enc_b.push_back(enc_tmp_b);
    }
    auto begin_time=clock();
    auto begin_time_high_precesion=clock_start();
    auto begin_bytes=gen->io->counter;
    vector<NTL::ZZ>r;
    vector<NTL::ZZ>enc_r;
    vector<NTL::ZZ>constant_2l;
    for(int i=0;i<1000;++i)
    {
        NTL::ZZ tmp_r=NTL::RandomLen_ZZ(106);
        NTL::ZZ tmp_enc_r=sp.encrypt(tmp_r);
        r.push_back(tmp_r);
        enc_r.push_back(tmp_enc_r);
        constant_2l.push_back(sp.encrypt(NTL::power_ZZ(2,64)));
    }
    int size=1000;
    int feature_bit=64;
    Batcher batcher1,batcher2;
    Integer*A=new Integer[size];
    Bit *B=new Bit[size];
    block *input_label0_alice=new block[size*feature_bit];
    block *input_label0_bob=new block[size*feature_bit];
    bool *di=new bool[size];
    gen->shared_prg.random_block(input_label0_bob,size*feature_bit);
    gen->shared_prg.random_block(input_label0_alice,size*feature_bit);
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
    }
    batcher1.make_semi_honest(BOB);
    batcher2.make_semi_honest(ALICE);//给label
    gen->input_label_alice=gen->input_label_alice-size*feature_bit;
    gen->input_label_bob=gen->input_label_bob-size*feature_bit;

    FileIO*iowrite =new FileIO("circuits.txt",0);
    gen->gc->set_genfile_io(iowrite);
    for(int i=0;i<size;++i)
    {
        B[i]=batcher1.next<Integer>() .geq(batcher2.next<Integer>()) ;
        di[i]=getLSB(B[i].bit);
    }
    cout<<"start point "<<m128i_to_string(gen->gc->start_point)<<endl;//hash nextindex 计数器
    cout<<"circuits gates table bytes "<<gen->gc->genfile->bytes_sent<<endl;
    iowrite->flush();
    delete iowrite;
    gen->di=di;//di 用不用到
    //cout<<gen->gc->io->counter<<endl;
    vector<bool>dec_d;
    vector<NTL::ZZ>enc_dec_d;
    for(int i=0;i<size;++i)
    {
        bool tmp= (r[i]>>64)%2;
        dec_d.push_back(di[i]^tmp);
        enc_dec_d.push_back(sp.encrypt(NTL::ZZ(di[i]^tmp)));
    }
    FileIO * ioread =new FileIO("circuits.txt",1);
    block * tables= new block[size*128];
    int table_size=size*128;
    ioread->recv_data(tables,table_size* sizeof(block));
    ioread->flush();
    delete ioread;
    delete[] A;
    delete[] B;
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
    vector<NTL::ZZ>enc_a_b;
    for(int i=0;i<1000;++i)
    {
        NTL::ZZ b_a=NTL::MulMod(enc_b[i],NTL::PowerMod(enc_a[i],-1,sp.modulus*sp.modulus),sp.modulus*sp.modulus);
        NTL::ZZ ans=NTL::MulMod(constant_2l[i],b_a,sp.modulus*sp.modulus);
        enc_a_b.push_back(ans);
    }
    NTL::ZZ expand_factor=NTL::PowerMod(NTL::ZZ(2),106,sp.modulus);
    for(int i=0;i<num_of_package-1;++i)
    {
        NTL::ZZ tmp=NTL::MulMod(enc_a_b[i*9+8],enc_r[i*9+8],sp.modulus*sp.modulus);

        for(int j=7;j>=0;--j)
        {
            NTL::ZZ t=NTL::MulMod(enc_a_b[i*9+j],enc_r[i*9+j],sp.modulus*sp.modulus);
            tmp=NTL::MulMod(NTL::PowerMod(tmp,expand_factor,sp.modulus*sp.modulus),t,sp.modulus*sp.modulus);
        }
        NTL::ZZ t2=sp.PDO(tmp);
        blind_C.push_back(tmp);
        blind_C1.push_back(t2);
    }
    if(last_package_num!=0){
        NTL::ZZ tmp=NTL::MulMod(enc_a_b[(num_of_package-1)*9+last_package_num-1],enc_r[(num_of_package-1)*9+last_package_num-1],sp.modulus*sp.modulus);
        for(int i=last_package_num-2;i>=0;--i)
        {
            NTL::ZZ t=NTL::MulMod(enc_a_b[(num_of_package-1)*9+i],enc_r[(num_of_package-1)*9+i],sp.modulus*sp.modulus);
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
    for(int i=0;i<1000;++i)
    {
        NTL::ZZ b_a=NTL::MulMod(enc_b[i],NTL::PowerMod(enc_a[i],-1,sp.modulus*sp.modulus),sp.modulus*sp.modulus);
        NTL::ZZ ans=NTL::MulMod(constant_2l[i],b_a,sp.modulus*sp.modulus);
        NTL::ZZ t=NTL::MulMod(ans,enc_r[i],sp.modulus*sp.modulus);
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

    vector<long>blind_mod_r;
    NTL::ZZ ring=NTL::power_ZZ(2,64);
    for(auto tmp:r) blind_mod_r.push_back(NTL::to_long(tmp%ring)+(uint64_t)pow(2,63));
    Integer R[size];//alice r
    //Integer bb[size];//alice 阈值t
    for(int i=0;i<size;++i)
    {
        R[i]=Integer (feature_bit,blind_mod_r[i],ALICE);
    }
    blind_mod_r.clear();
    block *r_lable=new block[size*feature_bit];
    for(int i=0;i<size;++i)
    {
        memcpy(r_lable+i*feature_bit,(block*)R[i].bits,feature_bit* sizeof(block));
        //memcpy(b_lable+i*feature_bit,(block*)bb[i].bits,feature_bit* sizeof(block));
    }

    gen->io->send_data(&size, sizeof(int));
    cout<<"size bytes :"<<gen->io->counter-begin_bytes<<endl;

    gen->io->send_block(r_lable,size*feature_bit);
    //gen->io->send_block(b_lable,size*feature_bit);
    cout<<"alice send her labels bytes "<<gen->io->counter-begin_bytes<<endl;
    delete[] r_lable;
    //delete[] b_lable;

    gen->io->send_data(&table_size, sizeof(int));
    cout<<"table_size bytes: "<<gen->io->counter-begin_bytes<<endl;
    gen->io->send_block(tables,table_size);
    gen->io->flush();
    cout<<"alice send tables bytes "<<gen->io->counter-begin_bytes<<endl;

    gen->ot->send_impl(input_label0_bob,input_label1_bob,size*feature_bit);
    gen->io->flush();
    cout<<"ot bytes "<<gen->io->counter-begin_bytes<<endl;

    delete[] input_label0_alice;
    delete[] input_label0_bob;
    delete[] input_label1_bob;
    delete[] tables;

    vector<NTL::ZZ>blind_result;
    long ciper_bytes=NTL::NumBytes(sp.modulus)*2;
    auto*p=new unsigned char[size*ciper_bytes];
    io->recv_data(p,size*ciper_bytes);
    io->flush();
    for(int i=0;i<size;++i)
    {
        NTL::ZZ tmp=NTL::ZZFromBytes(p+i*ciper_bytes,ciper_bytes);
        blind_result.push_back(tmp);
    }
    delete []p;
    vector<int>cmp;
    vector<NTL::ZZ>cmp_enc;
    for(int i=0;i<size;++i)
    {   cout<<dec_d[i]<<endl;
        NTL::ZZ tmp = NTL::MulMod(NTL::PowerMod(blind_result[i], NTL::ZZ(1 - 2 * dec_d[i]), sp.modulus *sp. modulus),
                                  enc_dec_d[i], sp.modulus * sp.modulus);
        cmp_enc.push_back(tmp);

    }
    auto all_end=clock();
    auto all_end_high=time_from(online_start_high);
    for(int i=0;i<size;++i)
    {
        cmp.push_back(NTL::to_int(auth.decrypt(cmp_enc[i])));
    }
    for(int i=0;i<size;++i)
    {
        cout<<a[i]<<" > "<<b[i]<<" "<<cmp[i]<<endl;
    }
    cout<<"offline time"<<(offline_time-begin_time)*1000/CLOCKS_PER_SEC<<endl;
    cout<<"offline time high "<<(long)offline_time_high/1000<<endl;
    cout<<"online time "<<(all_end-online_start)*1000/CLOCKS_PER_SEC<<endl;
    cout<<"online time high "<<(long)all_end_high/1000<<endl;
    cout<<(io->counter-begin_bytes)/1000<<endl;
    return 0;
}