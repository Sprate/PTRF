//
// Created by qhh on 2020/6/20.
//
#include "bob_eva.h"
using  namespace std;
using  namespace emp;
using  namespace NTL;

void eva_circuit(SemiHonestEva<NetIO>*eva,CSP &csp){
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

    csp.Step2();
    NTL::ZZ constant=NTL::PowerMod(NTL::ZZ(2),64,csp.modulus);
    vector<NTL::ZZ> blind_plain=csp.getBlindPlain();
    Integer b[size];
    Integer y[size];
    Integer a[size];
    Integer c[size];
    Bit result[size];
    block a_label[size*64];
    bool a_bool[size*64];

    for(int i=0;i<size;++i)
    {   NTL::ZZ beta=blind_plain[i]%constant;
        Integer::bool_data(a_bool+i*64,64,NTL::to_ulong(beta));
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
        //y[i]=Integer (64,y_label+i*64);
        a[i]=Integer (64,a_label+i*64);
        result[i]=b[i].geq(a[i]);
    }
    cout<<"eva time "<<time_from(start)<<endl;

    //eva->io->recv_data(di,size);

    /*for(int i=0;i<32;++i)
    {   cout<<"a b y"<<endl;
        cout<<m128i_to_string(a[i].bit)<<endl;
        cout<<m128i_to_string(b[i].bit)<<endl;
        cout<<m128i_to_string(y[i].bit)<<endl;
    }*/
    //cout<<m128i_to_string(c[6].bit)<<endl;
    //eva->io->recv_data(di,size);
    //eva->di=di;
    bool *result_bool=new bool[size];
    for(int i=0;i<size;++i)
    {
        // cout<<"result ::"<<m128i_to_string(result[i].bit)<<endl;
        //cout<<"table is "<<m128i_to_string(tables[6])<<endl;
       //result_bool[i]=result[i].reveal(BOB);
        result_bool[i]=getLSB(result[i].bit);
        cout<<"the result is::"<<result_bool[i]<<endl;
    }
    cout<<"reveal time "<<time_from(start)<<endl;
    cout<<"eva send all bytes "<<eva->io->counter-bytes<<endl;
    auto tt=clock_start();
    long ciper_bytes=csp.length_n*2;
    unsigned char*p=new unsigned char[size*ciper_bytes];
    for(int i=0;i<size;++i)
    {   bool rl= NTL::to_ulong(blind_plain[i]/constant)%2;
        ZZ tmp=csp.encrypt(ZZ(result_bool[i]^rl));
        //cout<<tmp<<endl;
        BytesFromZZ(p+i*ciper_bytes,tmp,ciper_bytes);
    }
    eva->io->send_data(p,size*ciper_bytes);
    eva->io->flush();
    cout<<" cmp rersult transform time is "<<time_from(tt)<<endl;
    delete []p;
    delete[] b_label;
    //delete[] y_label;
    delete[] di;
    delete[]tables;
    delete ioread;
    delete[] result_bool;

}
void eva_circuit_plain(SemiHonestEva<NetIO>*eva,CSP &csp){
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

    csp.Step2();

    vector<NTL::ZZ> blind_plain=csp.getBlindPlain();
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
    //eva->ot->setup_recv();
    //cout<<"base ot time "<<time_from(start)<<endl;
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

    //eva->io->recv_data(di,size);

    /*for(int i=0;i<32;++i)
    {   cout<<"a b y"<<endl;
        cout<<m128i_to_string(a[i].bit)<<endl;
        cout<<m128i_to_string(b[i].bit)<<endl;
        cout<<m128i_to_string(y[i].bit)<<endl;
    }*/
    //cout<<m128i_to_string(c[6].bit)<<endl;
    //eva->io->recv_data(di,size);
    //eva->di=di;
    bool *result_bool=new bool[size];
    for(int i=0;i<size;++i)
    {
        // cout<<"result ::"<<m128i_to_string(result[i].bit)<<endl;
        //cout<<"table is "<<m128i_to_string(tables[6])<<endl;
        //result_bool[i]=result[i].reveal(BOB);
        result_bool[i]=getLSB(result[i].bit);
        cout<<"the result is::"<<result_bool[i]<<endl;
    }
    cout<<"reveal time "<<time_from(start)<<endl;
    cout<<"eva send all bytes "<<eva->io->counter-bytes<<endl;
    auto tt=clock_start();
    long ciper_bytes=csp.length_n*2;
    unsigned char*p=new unsigned char[size*ciper_bytes];
    for(int i=0;i<size;++i)
    {
        ZZ tmp=csp.encrypt(ZZ(result_bool[i]));
        //cout<<tmp<<endl;
        BytesFromZZ(p+i*ciper_bytes,tmp,ciper_bytes);
    }
    eva->io->send_data(p,size*ciper_bytes);
    eva->io->flush();
    cout<<" cmp rersult transform time is "<<time_from(tt)<<endl;
    delete []p;
    delete[] b_label;
    //delete[] y_label;
    delete[] di;
    delete[]tables;
    delete ioread;
    delete[] result_bool;

}
void eva_circuit_encryptd(SemiHonestEva<NetIO>*eva,CSP &csp){
    uint64_t bytes=eva->io->counter;
    int size;
    auto start=clock_start();
    int table_size;
    cout<<"start point is"<<m128i_to_string(eva->gc->start_point)<<endl;
    eva->io->recv_data(&size, sizeof(int));
    block *b_label=new block[size*64];
    bool *di=new bool[size];

    eva->io->recv_block(b_label,size*64);
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

    csp.Step2();

    vector<NTL::ZZ> blind_plain=csp.getBlindPlain();
    Integer b[size];
    Integer a[size];
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
    //eva->ot->setup_recv();
    //cout<<"base ot time "<<time_from(start)<<endl;
    cout<<"before ot bytes "<<eva->io->counter-bytes<<endl;
    eva->ot->recv_impl(a_label,a_bool,size*64);
    cout<<"ot bytes "<<eva->io->counter-bytes<<endl;
    cout<<"ot time"<<time_from(start)<<endl;

    for(int i=0;i<size;++i)
    {
        b[i]=Integer (64,b_label+i*64);
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
    long ciper_bytes=csp.length_n*2;
    unsigned char*p=new unsigned char[size*ciper_bytes];
    for(int i=0;i<size;++i)
    {
        ZZ tmp=csp.encrypt(ZZ(result_bool[i]^z_l_1[i]));
        BytesFromZZ(p+i*ciper_bytes,tmp,ciper_bytes);
    }
    eva->io->send_data(p,size*ciper_bytes);
    eva->io->flush();
    cout<<" cmp rersult transform time is "<<time_from(tt)<<endl;
    delete []p;
    delete[] b_label;
    //delete[] y_label;
    delete[] di;
    delete[]tables;
    delete ioread;
    delete[] result_bool;

}
void initial_eva(SemiHonestEva<NetIO>*eva)
{
    block start_point=zero_block();
    eva->io->recv_block(&start_point,1);
    cout<<13232<<endl;
    cout<<m128i_to_string(start_point)<<endl;
    eva->gc->mitccrh.setS(start_point);
    eva->gc->start_point=start_point;
    block seed;
    eva->io->recv_block(&seed,1);
    eva->shared_prg.reseed(&seed);
    cout<<"init"<<endl;
    eva->io->flush();
}