//
// Created by Administrator on 2019/11/26.
//

#include "CSP.h"
#include<sstream>
#include "emp-sh2pc/bob_eva.h"
NTL::ZZ CSP::PDT(const NTL::ZZ &cipertext, const NTL::ZZ &C1) {
    clock_t start,end;
    start=clock();
    NTL::ZZ C2=PowerMod(cipertext,lambda2,modulus*modulus);
    NTL::ZZ e=MulMod(C1,C2,modulus*modulus);
    NTL::ZZ m=L_function(e);
    end=clock();
    cout<<"PDT one time run time is "<<(end-start)*1000000/CLOCKS_PER_SEC<<" us"<<endl;
    return m;
}

CSP::CSP( Paillier &p) {
    this->lambda2=p.GetLambda2();
    this->modulus=p.modulus;
    this->generator=p.generator;
}

void CSP::Step2() {
    //vector<pair<NTL::ZZ,NTL::ZZ>> rec_ciper_pair;
    clock_t start,end;
    start=clock();
    //depacking
    NTL::ZZ expand_factor=NTL::PowerMod(NTL::ZZ(2),106,modulus);
    for(vector<pair<NTL::ZZ,NTL::ZZ>>::size_type i=0;i<rec_ciper_pair.size()-1;++i)
    {
        NTL::ZZ tmp;
        tmp=PDT(rec_ciper_pair[i].first,rec_ciper_pair[i].second);
        for(int j=0;j<9;++j)
        {
            NTL::ZZ t=tmp%expand_factor;
            tmp=tmp/expand_factor;
            blind_plain.push_back(t);
            //cout<<tmp<<endl;
        }
    }
    NTL::ZZ tmp=PDT(rec_ciper_pair[rec_ciper_pair.size()-1].first,rec_ciper_pair[rec_ciper_pair.size()-1].second);
    if(last_package_num==0)
    {
        for(int j=0;j<9;++j)
        {
            NTL::ZZ t=tmp%expand_factor;
            tmp=tmp/expand_factor;
            blind_plain.push_back(t);
            //cout<<tmp<<endl;
        }
    }
    if(last_package_num!=0)
    {
        for(int j=0;j<last_package_num;++j)
        {
            NTL::ZZ t=tmp%expand_factor;
            tmp=tmp/expand_factor;
            blind_plain.push_back(t);
            //cout<<tmp<<endl;
        }

    }
    /*for(vector<pair<NTL::ZZ,NTL::ZZ>>::size_type i=0;i<rec_ciper_pair.size();++i)
    {
       NTL::ZZ tmp;
       tmp=PDT(rec_ciper_pair[i].first,rec_ciper_pair[i].second);
       blind_plain.push_back(NTL::to_long(tmp));
       cout<<tmp<<endl;
    }*/
    end=clock();
    cout<<"step2 run time is "<<(end-start)*1000000/CLOCKS_PER_SEC<<" us"<<endl;
}

void CSP::recvKey(NetIO * io) {
    long length_n;
    long length_lambda;
    io->recv_data(&length_n, sizeof(long));
    io->recv_data(&length_lambda, sizeof(long));
    unsigned char *recv_modulus=new unsigned char[length_n];
    unsigned char *recv_generator=new unsigned char[length_n];
    unsigned char *recv_lambda2=new unsigned char[length_lambda];
    io->recv_data(recv_modulus,length_n);
    io->recv_data(recv_generator,length_n);
    io->recv_data(recv_lambda2,length_lambda);
    this->modulus=ZZFromBytes(recv_modulus,length_n);
    this->generator=ZZFromBytes(recv_generator,length_n);
    this->SetLambda2(ZZFromBytes(recv_lambda2,length_lambda));
    this->length_n=length_n;
    delete[]  recv_modulus;
    delete [] recv_generator;
    delete [] recv_lambda2;
}

void CSP::recvCiper(NetIO *io) {
    long ciper_pair_size=0;
    long ciper_bytes=this->length_n*2;
    io->recv_data(&ciper_pair_size, sizeof(long));
    num_of_pakage=ciper_pair_size;
    io->recv_data(&last_package_num, sizeof(int));
    unsigned char *C=new unsigned char[ciper_pair_size*ciper_bytes];
    unsigned char *C1=new unsigned char[ciper_pair_size*ciper_bytes];
    io->recv_data(C,ciper_pair_size*ciper_bytes);
    io->recv_data(C1,ciper_pair_size*ciper_bytes);
    //vector<pair<NTL::ZZ,NTL::ZZ>> ciper_pair;
    for(int i=0;i<ciper_pair_size;++i)
    {
        ZZ c=ZZFromBytes(C+i*ciper_bytes,ciper_bytes);
        ZZ c1=ZZFromBytes(C1+i*ciper_bytes,ciper_bytes);
        rec_ciper_pair.push_back(make_pair(c,c1));
    }
    delete []C;
    delete []C1;
}
void CSP::step4(NetIO *io)
{
    long ciper_pair_size=0;
    long ciper_bytes=this->length_n*2;
    io->recv_data(&ciper_pair_size, sizeof(long));
    io->recv_data(&last_package_num, sizeof(int));
    auto t=clock_start();
    auto *C=new unsigned char[ciper_pair_size*ciper_bytes];
    auto *C1=new unsigned char[ciper_pair_size*ciper_bytes];
    io->recv_data(C,ciper_pair_size*ciper_bytes);
    io->recv_data(C1,ciper_pair_size*ciper_bytes);
    //vector<pair<NTL::ZZ,NTL::ZZ>> ciper_pair;
    for(int i=0;i<ciper_pair_size/2;++i)
    {
        ZZ pcj=ZZFromBytes(C+2*i*ciper_bytes,ciper_bytes);
        ZZ pcj_c1=ZZFromBytes(C1+2*i*ciper_bytes,ciper_bytes);
        cout<<pcj<<endl;
        ZZ vj=ZZFromBytes(C+(2*i+1)*ciper_bytes,ciper_bytes);
        ZZ vj_c1=ZZFromBytes(C1+(2*i+1)*ciper_bytes,ciper_bytes);
        cout<<vj_c1<<endl;
        pcj_vj.push_back(make_pair(make_pair(pcj,pcj_c1),make_pair(vj,vj_c1)));
    }
    delete []C;
    delete []C1;
    io->flush();
    cout<<"recvv pcj and vj time is "<<time_from(t)<<endl;

    //depacking
    NTL::ZZ expand_factor=NTL::PowerMod(NTL::ZZ(2),106,modulus);
    int exit=0;
    int index=-1;
    for(long i=0;i<pcj_vj.size();++i)
    {
        if(i!=pcj_vj.size()-1)
        {
            NTL::ZZ pack_pcj=PDT(pcj_vj[i].first.first,pcj_vj[i].first.second);
            for(int j=0;j<9;++j)
            {
                NTL::ZZ pcj=pack_pcj%expand_factor;
                pack_pcj=pack_pcj/expand_factor;
                if(pcj==ZZ(0))
                {   index=j;
                    exit=1;
                    break;
                }
            }
            if(exit==1)
            {
                NTL::ZZ pack_vj=PDT(pcj_vj[i].second.first,pcj_vj[i].second.second);
                NTL::ZZ vj;
                for(int k=0;k<=index;++k)
                {
                    vj=pack_vj%expand_factor;
                    pack_vj=pack_vj/expand_factor;
                }
                auto *cc=new unsigned char[NumBytes(vj)];
                BytesFromZZ(cc,vj,NumBytes(vj));
                io->send_data(cc, NumBytes(vj));
                delete [] cc ;
                break;
            }
        }
        if(i==pcj_vj.size()-1)
        {
            if(last_package_num==0)
            {
                NTL::ZZ pack_pcj=PDT(pcj_vj[i].first.first,pcj_vj[i].first.second);
                for(int j=0;j<9;++j)
                {
                    NTL::ZZ pcj=pack_pcj%expand_factor;
                    pack_pcj=pack_pcj/expand_factor;
                    if(pcj==ZZ(0))
                    {   index=j;
                        exit=1;
                        break;
                    }
                }
                if(exit==1)
                {
                    NTL::ZZ pack_vj=PDT(pcj_vj[i].second.first,pcj_vj[i].second.second);
                    NTL::ZZ vj;
                    for(int k=0;k<=index;++k)
                    {
                        vj=pack_vj%expand_factor;
                        pack_vj=pack_vj/expand_factor;
                    }
                    auto *cc=new unsigned char[NumBytes(vj)];
                    BytesFromZZ(cc,vj,NumBytes(vj));
                    io->send_data(cc, NumBytes(vj));
                    delete [] cc ;
                    break;
                }
            }
            if(last_package_num!=0)
            {
                NTL::ZZ pack_pcj=PDT(pcj_vj[i].first.first,pcj_vj[i].first.second);
                for(int j=0;j<last_package_num;++j)
                {
                    NTL::ZZ pcj=pack_pcj%expand_factor;
                    pack_pcj=pack_pcj/expand_factor;
                    if(pcj==ZZ(0))
                    {   index=j;
                        exit=1;
                        break;
                    }
                }
                if(exit==1)
                {
                    NTL::ZZ pack_vj=PDT(pcj_vj[i].second.first,pcj_vj[i].second.second);
                    NTL::ZZ vj;
                    for(int k=0;k<=index;++k)
                    {
                        vj=pack_vj%expand_factor;
                        pack_vj=pack_vj/expand_factor;
                    }
                    auto *cc=new unsigned char[NumBytes(vj)];
                    BytesFromZZ(cc,vj,NumBytes(vj));
                    io->send_data(cc, NumBytes(vj));
                    delete [] cc ;
                    break;
                }
            }
        }
    }
    /*for(auto &it :pcj_vj)
    {
        long pcj=to_long(PDT(it.first.first,it.first.second));
        if(pcj==0)
        {
            long vj=to_long(PDT(it.second.first,it.second.second));
            io->send_data(&vj, sizeof(long));
            break;
        }
    }*/
    cout<<"return result  time is"<<time_from(t)<<endl;
}

void CSP::clearCsp() {
    rec_ciper_pair.clear();
    blind_plain.clear();
    pcj_vj.clear();
    last_package_num=0;
    num_of_pakage=0;
}
inline string zz_to_string(NTL::ZZ &z)
{
    stringstream buffer;
    buffer<<z;
    return buffer.str();
}
void CSP::train_step(NetIO *io,SemiHonestEva<NetIO>*eva) {
    long ciper_pair_size=0;
    long ciper_bytes=this->length_n*2;
    io->recv_data(&ciper_pair_size, sizeof(long));
    unsigned char *C=new unsigned char[ciper_pair_size*ciper_bytes];
    unsigned char *C1=new unsigned char[ciper_pair_size*ciper_bytes];
    int *label_index=new int[ciper_pair_size*2];
    io->recv_data(C,ciper_pair_size*ciper_bytes);
    io->recv_data(C1,ciper_pair_size*ciper_bytes);
    io->recv_data(label_index,ciper_pair_size*2* sizeof(int));
    vector<pair<pair<NTL::ZZ,NTL::ZZ>,pair<int,int>>> ciper_pair;
    vector<pair<NTL::ZZ,pair<int,int>>>blind_sort_plain;
    for(int i=0;i<ciper_pair_size;++i)
    {
        ZZ c=ZZFromBytes(C+i*ciper_bytes,ciper_bytes);
        ZZ c1=ZZFromBytes(C1+i*ciper_bytes,ciper_bytes);
        int tmp_label=label_index[2*i];
        int tmp_index=label_index[2*i+1];
        ciper_pair.emplace_back(make_pair(c,c1),make_pair(tmp_label,tmp_index));
    }
    delete []C;
    delete []C1;
    io->flush();
    for(auto &ciper:ciper_pair)
    {
        NTL::ZZ tmp=PDT(ciper.first.first,ciper.first.second);
        blind_sort_plain.emplace_back(tmp,ciper.second);
    }

    uint64_t bytes=eva->io->counter;
    int size;
    auto start=clock_start();
    long table_size;
    cout<<"start point is"<<m128i_to_string(eva->gc->start_point)<<endl;
    eva->io->recv_data(&size, sizeof(int));
    block *b_label=new block[size*64];
    block *index_i_label=new block[size*64];

    bool *send_di=new bool[size*64];

    eva->io->recv_block(b_label,size*64);
    eva->io->recv_block(index_i_label,size*64);

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

    Integer b[size];
    Integer a[size];
    Integer c[size];
    Integer index_i[size];
    block a_label[size*64];
    bool a_bool[size*64];
    NTL::ZZ ring=NTL::power_ZZ(2,64);
    for(int i=0;i<size;++i)
    {
        Integer::bool_data(a_bool+i*64,64,NTL::to_long(blind_sort_plain[i].first%ring));
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
        index_i[i]=Integer(64,index_i_label+i*64);
        c[i]=a[i]-b[i];
        //c[i].resize(64,1);
    }
    sort(c,size,index_i);
    cout<<"sort"<<endl;
    cout<<"eva time "<<time_from(start)<<endl;

    //eva->io->recv_data(di,size);

    /*for(int i=0;i<32;++i)
    {   cout<<"a b y"<<endl;
        cout<<m128i_to_string(a[i].bit)<<endl;
        cout<<m128i_to_string(b[i].bit)<<endl;
        cout<<m128i_to_string(y[i].bit)<<endl;
    }*/
    //cout<<m128i_to_string(c[6].bit)<<endl;
    eva->io->recv_data(send_di,size*64 );
    eva->di=send_di;
    for(int i=0;i<size;++i) cout<<send_di[i]<<ends;
    cout<<endl;
    int64_t *sort_result =new int64_t[size];
    for(int i=0;i<size;++i)
    {
        // cout<<"result ::"<<m128i_t
        // o_string(result[i].bit)<<endl;
        //cout<<"table is "<<m128i_to_string(tables[6])<<endl;
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
    initial_eva(eva);
    cout<<" sort result transform time is "<<time_from(tt)<<endl;

    delete[] b_label;
    delete[] send_di;
    delete[]tables;
    delete ioread;
    delete[] sort_result;

}
void CSP::train_step_2(NetIO *io,SemiHonestEva<NetIO>*eva) {
    long ciper_pair_size=0;
    long ciper_bytes=this->length_n*2;
    int package_num=0;
    io->recv_data(&ciper_pair_size, sizeof(long));
    io->recv_data(&package_num, sizeof(int));
    unsigned char *C=new unsigned char[ciper_pair_size*ciper_bytes];
    unsigned char *C1=new unsigned char[ciper_pair_size*ciper_bytes];
    io->recv_data(C,ciper_pair_size*ciper_bytes);
    io->recv_data(C1,ciper_pair_size*ciper_bytes);
    vector<pair<NTL::ZZ,NTL::ZZ>> ciper_pair;
    vector<NTL::ZZ>blind_sort_plain;
    for(int i=0;i<ciper_pair_size;++i)
    {
        ZZ c=ZZFromBytes(C+i*ciper_bytes,ciper_bytes);
        ZZ c1=ZZFromBytes(C1+i*ciper_bytes,ciper_bytes);
        ciper_pair.emplace_back(make_pair(c,c1));
    }
    delete []C;
    delete []C1;
    io->flush();
    NTL::ZZ expand_factor=NTL::PowerMod(NTL::ZZ(2),106,modulus);
    for(vector<pair<NTL::ZZ,NTL::ZZ>>::size_type i=0;i<ciper_pair.size()-1;++i)
    {
        NTL::ZZ tmp;
        tmp=PDT(ciper_pair[i].first,ciper_pair[i].second);
        for(int j=0;j<9;++j)
        {
            NTL::ZZ t=tmp%expand_factor;
            tmp=tmp/expand_factor;
            blind_sort_plain.push_back(t);
            //cout<<tmp<<endl;
        }
    }
    NTL::ZZ tmp=PDT(ciper_pair[ciper_pair.size()-1].first,ciper_pair[ciper_pair.size()-1].second);
    if(package_num==0)
    {
        for(int j=0;j<9;++j)
        {
            NTL::ZZ t=tmp%expand_factor;
            tmp=tmp/expand_factor;
            blind_sort_plain.push_back(t);
        }
    }
    if(package_num!=0)
    {
        for(int j=0;j<package_num;++j)
        {
            NTL::ZZ t=tmp%expand_factor;
            tmp=tmp/expand_factor;
            blind_sort_plain.push_back(t);
            //cout<<tmp<<endl;
        }

    }

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

    Integer b[size];
    Integer a[size];
    Integer c[size];
    Integer index_i[size];
    block a_label[size*feature_bit];
    bool a_bool[size*feature_bit];
    NTL::ZZ ring=NTL::power_ZZ(2,64);
    for(int i=0;i<size;++i)
    {
        Integer::bool_data(a_bool+i*feature_bit,feature_bit,NTL::to_long(blind_sort_plain[i]%ring));
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

    eva->io->recv_data(send_di,size*index_bit );
    eva->di=send_di;
    for(int i=0;i<size;++i) cout<<send_di[i]<<ends;
    cout<<endl;
    int64_t *sort_result =new int64_t[size];
    for(int i=0;i<size;++i)
    {
        // cout<<"result ::"<<m128i_t
        // o_string(result[i].bit)<<endl;
        //cout<<"table is "<<m128i_to_string(tables[6])<<endl;
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
    initial_eva(eva);
    cout<<" sort result transform time is "<<time_from(tt)<<endl;

    delete[] b_label;
    delete[] send_di;
    delete[]tables;
    delete ioread;
    delete[] sort_result;

}

