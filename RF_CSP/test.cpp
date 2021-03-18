//
// Created by bonjour on 2021/1/2.
//
#include "emp-sh2pc/emp-sh2pc.h"
#include <NTL/ZZ.h>
#include "emp-sh2pc/bob_eva.h"
#include "CSP.h"

void test_circuit(SemiHonestEva<NetIO>*eva){
    uint64_t bytes=eva->io->counter;
    int size;
    auto start=clock_start();
    int table_size;
    cout<<"start point is"<<m128i_to_string(eva->gc->start_point)<<endl;
    eva->io->recv_data(&size, sizeof(int));
    block *b_label=new block[size*64];
    block *c_label=new block[size*10];
    bool *di=new bool[size*10];

    eva->io->recv_block(b_label,size*64);
    eva->io->recv_block(c_label,size*10);
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

    Integer b[size];
    Integer y[size];
    Integer a[size];
    Integer c[size];
    Integer result[size];
    block a_label[size*64];
    bool a_bool[size*64];

    for(int i=0;i<size;++i)
    {
        Integer::bool_data(a_bool+i*64,64,80-i);
    }
    cout<<"before ot time "<<time_from(start)<<endl;
    //eva->ot->setup_recv();
    //cout<<"base ot time "<<time_from(start)<<endl;
    cout<<"before ot bytes "<<eva->io->counter-bytes<<endl;
    eva->ot->recv_impl(a_label,a_bool,size*64);
    cout<<"ot bytes "<<eva->io->counter-bytes<<endl;
    cout<<"ot time"<<time_from(start)<<endl;

    for(int i=0;i<size;++i)
    {
        b[i]=Integer (64,b_label+i*64);
        c[i]=Integer (10,c_label+i*10);
        a[i]=Integer (64,a_label+i*64);
        y[i]=a[i]-b[i];
    }
    sort(y,size,c);
    cout<<"eva time "<<time_from(start)<<endl;

    //eva->io->recv_data(di,size);

    /*for(int i=0;i<64;++i)
    {   cout<<"a b y"<<endl;
        cout<<m128i_to_string(a[i].bit)<<endl;
        cout<<m128i_to_string(b[i].bit)<<endl;
        cout<<m128i_to_string(y[i].bit)<<endl;
    }*/
    //cout<<m128i_to_string(c[6].bit)<<endl;
    eva->io->recv_data(di,size*10);
    eva->di=di;
    bool *result_bool=new bool[size];
    for(int i=0;i<size;++i)
    {
        // cout<<"result ::"<<m128i_to_string(result[i].bit)<<endl;
        //cout<<"table is "<<m128i_to_string(tables[6])<<endl;
        //result_bool[i]=result[i].reveal(BOB);
        //result_bool[i]=getLSB(result[i].bit);
        //cout<<"the result is::"<<result_bool[i]<<endl;
        cout<<c[i].reveal<long>(BOB)<<endl;
    }
    cout<<"reveal time "<<time_from(start)<<endl;
    cout<<"eva send all bytes "<<eva->io->counter-bytes<<endl;


    delete[] b_label;
    //delete[] y_label;
    delete[] di;
    delete[]tables;
    delete ioread;
    delete[] result_bool;

}
int main() {
    int port, party;
    party=BOB;
    port=12345;
    NetIO * io = new NetIO(party==ALICE ? nullptr : "192.168.1.120", port);

    SemiHonestGen<NetIO> *gen= nullptr;
    SemiHonestEva<NetIO> *eva= nullptr;
    setup_semi_honest(io, party,gen,eva);
    cout<<"initial byets"<<io->counter<<endl;
    auto ot_start=clock_start();
    eva->ot->setup_recv();
    cout<<"base ot bytes "<<io->counter<<endl;
    cout<<"base ot time "<<time_from(ot_start)<<endl;
    io->flush();
    test_circuit(eva);
    delete eva;
    delete gen;
    delete io;

}