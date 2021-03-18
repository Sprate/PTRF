//
// Created by bonjour on 2020/2/15.
//

#ifndef RF_ALICE_SORT_H
#define RF_ALICE_SORT_H

#include "emp-sh2pc/emp-sh2pc.h"
#include <NTL/ZZ.h>
#include "SP.h"
using namespace emp;
using namespace std;
void gen_sort_circuits(int party,int size,SemiHonestGen<NetIO> *gen,SP &sp){
    auto start=clock_start();
    uint64_t bytes=gen->io->counter;
    int feature_bit=64;
    cout<<"before gen_circuits send bytes"<<bytes<<endl;
    Batcher batcher1,batcher2;
    Integer*A=new Integer[size];
    Bit *B=new Bit[size];
    block *input_label0_alice=new block[size*feature_bit];
    block *input_label0_bob=new block[size*feature_bit];
    bool *di=new bool[size];
    //PRG prg;
    gen->shared_prg.random_block(input_label0_bob,size*feature_bit);
    //prg.random_block(input_label0_bob,size*32);
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

    FileIO*iowrite =new FileIO("circuits_sort.txt",0);
    gen->gc->set_genfile_io(iowrite);

    for(int i=0;i<size;++i)
    {
        A[i]=batcher1.next<Integer>() - batcher2.next<Integer>();
        sort(A,size);
        for(int j=0;j<feature_bit;++j)
        {
            di[i*feature_bit+j]=getLSB(A[i][j].bit);
        }
        //B[i].reveal<string>(ALICE);
    }
    cout<<"start point "<<m128i_to_string(gen->gc->start_point)<<endl;//hash nextindex 计数器
    cout<<"circuits gates table bytes "<<gen->gc->genfile->bytes_sent<<endl;
    long table_size=gen->gc->genfile->bytes_sent/16;
    iowrite->flush();
    delete iowrite;
    gen->di=di;//di 用不用到
    //cout<<gen->gc->io->counter<<endl;
    double t=time_from(start);
    cout<<"offline time(input labels ,ciucuits gates):"<<t<<endl;

    FileIO * ioread =new FileIO("circuits_sort.txt",1);
    block * tables= new block[table_size];

    //cout<<456<<endl;
    ioread->recv_data(tables,table_size* sizeof(block));
    ioread->flush();
    delete ioread;

    //online begin
    cout<<"online begin "<<endl;
    vector<long>blind_r=sp.getBlindR();
    vector<long>thresholds=sp.getThreshold();
    Integer a[size];//bob y+r
    Integer b[size];//alice r
    Integer y[size];//alice 阈值t
    for(int i=0;i<size;++i)
    {//a[i]=Integer(32,15,BOB);
        // b[i]=Integer (32,5,ALICE);
        b[i]=Integer (feature_bit,blind_r[i],ALICE);
    }
    for(int i=0;i<size;++i)
    {
        y[i]=Integer(feature_bit,thresholds[i],ALICE);
    }
    double t2=time_from(start);
    cout<<"real input to garbled labels time "<<t2<<endl;
    /*for(int i=0;i<32;++i)
    {cout<<"a b y"<<endl;
        cout<<m128i_to_string(a[i].bit)<<endl;
        cout<<m128i_to_string(b[i].bit)<<endl;
        cout<<m128i_to_string(y[i].bit)<<endl;
    }*/
    /*for(int i=0;i<size;++i)
    {
        //cout<<m128i_to_string(A[i][6].bit)<<endl;
        //cout<<m128i_to_string(xorBlocks(A[i][6].bit,gen->gc->delta))<<endl;
        //cout<<"result 0:"<<m128i_to_string(B[i].bit)<<endl;
       // cout<<"result 1:"<<m128i_to_string(xorBlocks(B[i].bit,gen->gc->delta))<<endl;
    }*/

    //cout<<"table is "<<m128i_to_string(tables[6])<<endl;

    block *b_lable=new block[size*feature_bit];
    block *y_lable=new block[size*feature_bit];
    for(int i=0;i<size;++i)
    {
        memcpy(b_lable+i*feature_bit,(block*)b[i].bits,feature_bit* sizeof(block));
        //gen->io->send_block((block*)b[i].bits,32);
        memcpy(y_lable+i*feature_bit,(block*)y[i].bits,feature_bit* sizeof(block));
    }

    gen->io->send_data(&size, sizeof(int));
    cout<<"size bytes :"<<gen->io->counter-bytes<<endl;

    gen->io->send_block(b_lable,size*feature_bit);
    gen->io->send_block(y_lable,size*feature_bit);
    cout<<"alice send her labels bytes "<<gen->io->counter-bytes<<endl;
    delete[] b_lable;
    delete[] y_lable;
    cout<<"alice send input time "<<time_from(start)<<endl;

    gen->io->send_data(&table_size, sizeof(int));
    cout<<"table_size bytes: "<<gen->io->counter-bytes<<endl;
    gen->io->send_block(tables,table_size);
    gen->io->flush();
    cout<<"alice send tables bytes "<<gen->io->counter-bytes<<endl;
    long long t4=time_from(start);
    cout<<"alice send tables times "<<t4<<endl;
    //gen->ot->setup_send();
    // cout<<"bast ot "<<time_from(start)<<endl;
    gen->ot->send_impl(input_label0_bob,input_label1_bob,size*feature_bit);
    cout<<"ot bytes "<<gen->io->counter-bytes<<endl;
    cout<<" ot time "<<time_from(start)<<endl;
    bool *blind_di=new bool[size];
    vector<bool>blind_z=sp.getBlindZ();
    for(int i=0;i<size;++i)
    {
        blind_di[i]=di[i]^blind_z[i];
    }
    for(const auto a:blind_z)
    {
        cout<<a<<ends;
    }
    cout<<endl;
    blind_z.clear();
    gen->io->send_data(blind_di,size);
    //gen->io->send_data(di,size);
    cout<<gen->io->counter-bytes<<endl;
    gen->io->flush();
    cout<<"all time"<<time_from(start)<<endl;

    /*sp.recvCmpResult(gen->io);
    gen->io->flush();
    cout<<"recv cmp result time is "<<time_from(start)<<"us"<<endl;*/

    delete[] A;
    delete[] B;
    delete[] input_label0_alice;
    delete[] input_label0_bob;
    delete[] input_label1_bob;
    delete[] tables;
    delete[]blind_di;
}
/*
void init_circuits(SemiHonestGen<NetIO>  *gen)
{
    PRG tmp;
    tmp.random_block(&gen->gc->seed, 1);
    block a;
    tmp.random_block(&a, 1);
    gen->gc->set_delta(a);
    block start_point;
    tmp.random_block(&start_point, 1);
    gen->io->send_block(&start_point, 1);
    cout<<"start point"<<m128i_to_string(start_point)<<endl;
    gen->gc->mitccrh.setS(start_point);
    gen->gc->start_point=start_point;
    block seed;
    tmp.random_block(&seed, 1);
    gen->io->send_block(&seed, 1);
    gen->shared_prg.reseed(&seed);
}
 */
#endif //RF_ALICE_SORT_H
