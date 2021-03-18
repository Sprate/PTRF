//
// Created by qhh on 2021/1/2.
//

#include "alice_gen.h"
using namespace emp;
using namespace std;
void gen_circuits(int party,int size,SemiHonestGen<NetIO> *gen,SP &sp){
    auto start=clock_start();
    uint64_t bytes=gen->io->counter;
    int feature_bit=64;
    cout<<"before gen_circuits send bytes"<<bytes<<endl;
    Batcher batcher1,batcher2,batcher3;
    Integer*A=new Integer[size];
    Bit *B=new Bit[size];
    block *input_label0_alice=new block[size*feature_bit*2];
    block *input_label0_bob=new block[size*feature_bit];
    bool *di=new bool[size];
    //PRG prg;
    gen->shared_prg.random_block(input_label0_bob,size*feature_bit);
    //prg.random_block(input_label0_bob,size*32);
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
        //B[i].reveal<string>(ALICE);
    }
    cout<<"start point "<<m128i_to_string(gen->gc->start_point)<<endl;//hash nextindex 计数器
    cout<<"circuits gates table bytes "<<gen->gc->genfile->bytes_sent<<endl;
    iowrite->flush();
    delete iowrite;
    gen->di=di;//di 用不用到
    //cout<<gen->gc->io->counter<<endl;
    double t=time_from(start);
    vector<bool>dec_d;
    for(int i=0;i<size;++i)
    {
        dec_d.push_back(di[i]);
    }
    sp.setDecD(dec_d);
    sp.genEncDecD();
    cout<<"offline time(input labels ,ciucuits gates):"<<t<<endl;

    FileIO * ioread =new FileIO("circuits.txt",1);
    block * tables= new block[size*254];
    int table_size=size*254;
    //cout<<456<<endl;
    ioread->recv_data(tables,table_size* sizeof(block));
    ioread->flush();
    delete ioread;
    long long t1=time_from(start);
    cout<<"push back offline time "<<t1<<"us"<<endl;
    sp.offline_time.push_back(t1);

    //online begin
    cout<<"online begin "<<endl;
    vector<NTL::ZZ>blind_r=sp.getBlindR();
    vector<long>blind_mod_r;
    NTL::ZZ ring=NTL::power_ZZ(2,64);
    for(auto tmp:blind_r) blind_mod_r.push_back(NTL::to_long(tmp%ring));
    vector<long>thresholds=sp.getThreshold();
    Integer a[size];//bob y+r
    Integer b[size];//alice r
    Integer y[size];//alice 阈值t
    for(int i=0;i<size;++i)
    {//a[i]=Integer(32,15,BOB);
        // b[i]=Integer (32,5,ALICE);
        b[i]=Integer (feature_bit,blind_mod_r[i],ALICE);
    }
    blind_mod_r.clear();
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
    /*
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
     */
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
    //delete[]blind_di;
}
void gen_circuits_2(int party,int size,SemiHonestGen<NetIO> *gen,SP &sp){
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
    batcher2.make_semi_honest(ALICE);
    //给label
    gen->input_label_alice=gen->input_label_alice-size*feature_bit;
    gen->input_label_bob=gen->input_label_bob-size*feature_bit;

    FileIO*iowrite =new FileIO("circuits.txt",0);
    gen->gc->set_genfile_io(iowrite);
    for(int i=0;i<size;++i)
    {
        B[i]=batcher1.next<Integer>().geq(batcher2.next<Integer>());
        di[i]=getLSB(B[i].bit);
        //B[i].reveal<string>(ALICE);
    }
    cout<<"start point "<<m128i_to_string(gen->gc->start_point)<<endl;//hash nextindex 计数器
    cout<<"circuits gates table bytes "<<gen->gc->genfile->bytes_sent<<endl;
    iowrite->flush();
    delete iowrite;
    gen->di=di;//di 用不用到
    //cout<<gen->gc->io->counter<<endl;
    double t=time_from(start);
    vector<bool>dec_d;
    vector<NTL::ZZ>_r=sp.getBlindR();
    for(int i=0;i<size;++i)
    {   bool tmp=(_r[i]>>64)%2;
        dec_d.push_back(di[i]^tmp);
    }
    sp.setDecD(dec_d);
    sp.genEncDecD();
    cout<<"offline time(input labels ,ciucuits gates):"<<t<<endl;

    FileIO * ioread =new FileIO("circuits.txt",1);
    block * tables= new block[size*128];
    int table_size=size*128;
    //cout<<456<<endl;
    ioread->recv_data(tables,table_size* sizeof(block));
    ioread->flush();
    delete ioread;
    long long t1=time_from(start);
    cout<<"push back offline time "<<t1<<"us"<<endl;
    sp.offline_time.push_back(t1);

    //online begin
    cout<<"online begin "<<endl;
    vector<NTL::ZZ>blind_r=sp.getBlindR();
    vector<long>blind_mod_r;
    NTL::ZZ ring=NTL::power_ZZ(2,64);
    for(auto tmp:blind_r) blind_mod_r.push_back(NTL::to_long(tmp%ring)+(uint64_t)pow(2,63));
    Integer a[size];//bob z mod 2^l
    Integer b[size];//alice r mod 2^l
    for(int i=0;i<size;++i)
    {
        b[i]=Integer (feature_bit,blind_mod_r[i],ALICE);
    }
    blind_mod_r.clear();

    double t2=time_from(start);
    cout<<"real input to garbled labels time "<<t2<<endl;

    block *b_lable=new block[size*feature_bit];
    for(int i=0;i<size;++i)
    {
        memcpy(b_lable+i*feature_bit,(block*)b[i].bits,feature_bit* sizeof(block));
    }
    gen->io->send_data(&size, sizeof(int));
    cout<<"size bytes :"<<gen->io->counter-bytes<<endl;

    gen->io->send_block(b_lable,size*feature_bit);
    cout<<"alice send her labels bytes "<<gen->io->counter-bytes<<endl;
    delete[] b_lable;
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

    cout<<gen->io->counter-bytes<<endl;
    gen->io->flush();
    cout<<"all time"<<time_from(start)<<endl;

    delete[] A;
    delete[] B;
    delete[] input_label0_alice;
    delete[] input_label0_bob;
    delete[] input_label1_bob;
    delete[] tables;
    //delete[]blind_di;
}
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