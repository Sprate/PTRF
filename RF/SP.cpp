//
// Created by Administrator on 2019/11/26.
//

#include "SP.h"
#include <random>
#include <vector>
#include <NTL/ZZ.h>

NTL::ZZ SP::PDO(const NTL::ZZ &cipertext) {
    clock_t start,end;
    start=clock();
    NTL::ZZ Ci=PowerMod(cipertext,lambda1,modulus*modulus);
    //cout<<cipertext<<endl;
    //cout<<lambda1<<endl;
    end=clock();
    cout<<"PDO once run time is "<<(end-start)*1000000/CLOCKS_PER_SEC<<" us"<<endl;
    return Ci;
}

SP::SP(Paillier &p){
   this->lambda1=p.GetLambda1();
    this->modulus=p.modulus;
    this->generator=p.generator;
}

SP::SP() {
    this->lambda1=0;
    this->generator=0;
}

SP::~SP() {

}

vector<vector<int>> SP::genPermutationMatrix_2() {
    vector<vector<int>>tmp(enc_decision_nodes.size(),vector<int>(NumOfFeatures,0));
    int count=0;
    for(auto it=enc_decision_nodes.begin();it!=enc_decision_nodes.end();++it)
    {
        tmp[count][it->second.first]=1;
        enc_threshold.push_back(it->second.second);
        ++count;
    }
    return tmp;
}
vector<vector<int>> SP::genPermutationMatrix() {
    vector<vector<int>>tmp(decision_nodes.size(),vector<int>(NumOfFeatures,0));
    int count=0;
    for(auto it=decision_nodes.begin();it!=decision_nodes.end();++it)
    {
        tmp[count][it->second.first]=1;
        threshold.push_back(it->second.second);
        ++count;
    }
    return tmp;
}

vector<NTL::ZZ> SP::dataTransform(const vector<NTL::ZZ> &enc_features) {
    vector<NTL::ZZ>tmp;
    for(size_t i=0;i<matrix.size();++i)
    {
        NTL::ZZ y=NTL::PowerMod(enc_features[0],matrix[i][0],modulus*modulus);
        for(size_t j=1;j<matrix[i].size();++j)
        {
            y=NTL::MulMod(y,NTL::PowerMod(enc_features[j],matrix[i][j],modulus*modulus),modulus*modulus);
        }
        tmp.push_back(y);
    }
    return tmp;
}
inline vector<NTL::ZZ>genBlindR(int times){
    vector< NTL::ZZ>tmp;
    for(int i=0;i<times;++i)
    {
        NTL::ZZ a=NTL::RandomLen_ZZ(106);
        tmp.push_back(a);
    }
    return tmp;
}
inline vector<bool>genBlindZ(int times){
    vector<bool>tmp;
    for(int i=0;i<times;++i)
    {
        bool a=NTL::RandomBits_long(1);
        tmp.push_back(a);
    }
    return tmp;
}
void SP::genEncRi() {
    for(auto & it : blind_r)
    {
        NTL::ZZ tmp(it);
        enc_ri.push_back(encrypt(tmp));
    }
}
void SP::genEncZi() {
    for(auto it=blind_Z.begin();it!=blind_Z.end();++it)
    {
        NTL::ZZ tmp(*it);
        enc_Z.push_back(encrypt(tmp));
    }
}
void SP::genEncDecD() {
    for (auto it:dec_d)
    {
        NTL::ZZ tmp(it);
        enc_dec_d.push_back(encrypt(tmp));
    }
}

void SP::step1(const vector<NTL::ZZ>&enc_features) {
    clock_t start,end;
    start=clock();
    auto offline_time_start=clock_start();
    //map<int, pair<int, long>> decision_nodes;
    //vector<NTL::ZZ> enc_features;
    matrix=genPermutationMatrix();
    cout<<"begin gen ri and Z "<<endl;
    blind_r=genBlindR(m);
    //blind_Z=genBlindZ(m);
    genPathCostRAndValueR();
    genEncLeafValues();
    genEncRi();
    //genEncZi();
    cout<<"gen ri and Z completed "<<endl;
    cout<<"push offline time :gen matrix random r Z path_cost_r value_r enc_leaf_value enc_r enc_Z "<<time_from(offline_time_start)<<"us"<<endl;
    offline_time.push_back(time_from(offline_time_start));
    vector<NTL::ZZ> y=dataTransform(enc_features);
    vector<NTL::ZZ> blind_C1;
    cout<<"begin gen packing ciper pair "<<endl;
    //datapacking

    int num_of_package=y.size()/9+1;
    int last_package_num=y.size()%9;
    NTL::ZZ expand_factor=NTL::PowerMod(NTL::ZZ(2),106,modulus);
    for(int i=0;i<num_of_package-1;++i)
    {   NTL::ZZ tmp=NTL::MulMod(y[i*9+8],enc_ri[i*9+8],modulus*modulus);
        for(int j=7;j>=0;--j)
        {
            NTL::ZZ t=NTL::MulMod(y[i*9+j],enc_ri[i*9+j],modulus*modulus);
            tmp=NTL::MulMod(NTL::PowerMod(tmp,expand_factor,modulus*modulus),t,modulus*modulus);
        }
        NTL::ZZ t2=PDO(tmp);
        blind_C.push_back(tmp);
        blind_C1.push_back(t2);
        ciper_pair.emplace_back(tmp,t2);
    }
    if(last_package_num!=0){
        NTL::ZZ tmp=NTL::MulMod(y[(num_of_package-1)*9+last_package_num-1],enc_ri[(num_of_package-1)*9+last_package_num-1],modulus*modulus);
        for(int i=last_package_num-2;i>=0;--i)
        {
            NTL::ZZ t=NTL::MulMod(y[(num_of_package-1)*9+i],enc_ri[(num_of_package-1)*9+i],modulus*modulus);
            tmp=NTL::MulMod(NTL::PowerMod(tmp,expand_factor,modulus*modulus),t,modulus*modulus);
        }
        NTL::ZZ t2=PDO(tmp);
        blind_C.push_back(tmp);
        blind_C1.push_back(t2);
        ciper_pair.emplace_back(tmp,t2);
        package_information.first=num_of_package;
        package_information.second=last_package_num;
    }
    if(last_package_num==0)
    {
        package_information.first=num_of_package-1;
        package_information.second=last_package_num;
    }
    cout<<"num of package "<<package_information.first<<endl;
    cout<<"last package num "<<package_information.second<<endl;

    /*for(size_t i=0;i<y.size();++i)
    {
        NTL::ZZ t=NTL::MulMod(y[i],enc_ri[i],modulus*modulus);
        NTL::ZZ t1=PDO(t);
        blind_C.push_back(t);
        blind_C1.push_back(t1);
        ciper_pair.emplace_back(t,t1);
    }*/
    cout<<"gen packing ciper pair completed"<<endl;
    end=clock();
    cout<<"SP step 1 run time before io send ciper is "<<(end-start)*10000000/CLOCKS_PER_SEC<<" us"<<endl;
}

void SP::step1_2(const vector<NTL::ZZ>&enc_features) {
    clock_t start,end;
    start=clock();
    auto offline_time_start=clock_start();
    //map<int, pair<int, long>> decision_nodes;
    //vector<NTL::ZZ> enc_features;
    matrix=genPermutationMatrix_2();
    cout<<"begin gen ri and Z "<<endl;
    blind_r=genBlindR(m);
    //blind_Z=genBlindZ(m);
    genPathCostRAndValueR();
    genEncLeafValues();
    genEncRi();
    //genEncZi();
    cout<<"gen ri  completed "<<endl;
    cout<<"push offline time :gen matrix random r  path_cost_r value_r enc_leaf_value enc_r enc_Z "<<time_from(offline_time_start)<<"us"<<endl;
    offline_time.push_back(time_from(offline_time_start));
    vector<NTL::ZZ> _y=dataTransform(enc_features);
    vector<NTL::ZZ> blind_C1;
    cout<<"begin gen packing ciper pair "<<endl;
    //datapacking

    vector<NTL::ZZ> y;
    NTL::ZZ constant_2l=encrypt(NTL::power_ZZ(2,64));
    auto b_a_clock=clock_start();
    for(int i=0;i<_y.size();++i)
    {
        NTL::ZZ b_a=NTL::MulMod(enc_threshold[i],NTL::PowerMod(_y[i],-1,modulus*modulus),modulus*modulus);
        NTL::ZZ ans=NTL::MulMod(constant_2l,b_a,modulus*modulus);
        y.push_back(ans);
    }
    _y.clear();
    cout<<" N-1 cost time "<<time_from(b_a_clock)<<"us"<<endl;
    int num_of_package=y.size()/9+1;
    int last_package_num=y.size()%9;
    NTL::ZZ expand_factor=NTL::PowerMod(NTL::ZZ(2),106,modulus);
    for(int i=0;i<num_of_package-1;++i)
    {
        NTL::ZZ tmp=NTL::MulMod(y[i*9+8],enc_ri[i*9+8],modulus*modulus);
        for(int j=7;j>=0;--j)
        {
            NTL::ZZ t=NTL::MulMod(y[i*9+j],enc_ri[i*9+j],modulus*modulus);
            tmp=NTL::MulMod(NTL::PowerMod(tmp,expand_factor,modulus*modulus),t,modulus*modulus);
        }
        NTL::ZZ t2=PDO(tmp);
        blind_C.push_back(tmp);
        blind_C1.push_back(t2);
        ciper_pair.emplace_back(tmp,t2);
    }
    if(last_package_num!=0){
        NTL::ZZ tmp=NTL::MulMod(y[(num_of_package-1)*9+last_package_num-1],enc_ri[(num_of_package-1)*9+last_package_num-1],modulus*modulus);
        for(int i=last_package_num-2;i>=0;--i)
        {
            NTL::ZZ t=NTL::MulMod(y[(num_of_package-1)*9+i],enc_ri[(num_of_package-1)*9+i],modulus*modulus);
            tmp=NTL::MulMod(NTL::PowerMod(tmp,expand_factor,modulus*modulus),t,modulus*modulus);
        }
        NTL::ZZ t2=PDO(tmp);
        blind_C.push_back(tmp);
        blind_C1.push_back(t2);
        ciper_pair.emplace_back(tmp,t2);
        package_information.first=num_of_package;
        package_information.second=last_package_num;
    }
    if(last_package_num==0)
    {
        package_information.first=num_of_package-1;
        package_information.second=last_package_num;
    }
    cout<<"num of package "<<package_information.first<<endl;
    cout<<"last package num "<<package_information.second<<endl;

    /*for(size_t i=0;i<y.size();++i)
    {
        NTL::ZZ t=NTL::MulMod(y[i],enc_ri[i],modulus*modulus);
        NTL::ZZ t1=PDO(t);
        blind_C.push_back(t);
        blind_C1.push_back(t1);
        ciper_pair.emplace_back(t,t1);
    }*/
    cout<<"gen packing ciper pair completed"<<endl;
    end=clock();
    cout<<"SP step 1 run time before io send ciper is "<<(end-start)*10000000/CLOCKS_PER_SEC<<" us"<<endl;
}

void SP::sendCiper(NetIO * io) {
    long ciper_pair_size=ciper_pair.size();
    auto *C=new unsigned char[ciper_pair_size*256];
    auto *C1=new unsigned char[ciper_pair_size*256];

    for(long i=0;i<ciper_pair_size;++i)
    {
        NTL::BytesFromZZ(C+i*256,ciper_pair[i].first,256);
        NTL::BytesFromZZ(C1+i*256,ciper_pair[i].second,256);
    }
    io->send_data(&ciper_pair_size, sizeof(long));
    io->send_data(&package_information.second, sizeof(int));
    io->send_data(C,ciper_pair_size*256);
    io->send_data(C1,ciper_pair_size*256);
    delete []C;
    delete []C1;
}

void SP::recvCmpResult(NetIO *io)  {
    long ciper_bytes=NTL::NumBytes(modulus)*2;
    auto*p=new unsigned char[m*ciper_bytes];
    auto t=clock_start();
    io->recv_data(p,m*ciper_bytes);
    for(int i=0;i<m;++i)
    {
        NTL::ZZ tmp=NTL::ZZFromBytes(p+i*ciper_bytes,ciper_bytes);
        blind_cmp_result.push_back(tmp);
        cout<<tmp<<endl;
    }
    cout<<"time is "<<time_from(t)<<endl;
    delete []p;
}

void SP::step3(const NTL::ZZ& enc_tree_r,bool encrypted_threshold) {
    NTL::ZZ one = encrypt(NTL::ZZ(1));
    //得到edge_cost
    auto t1=clock_start();
    if(!encrypted_threshold)
    {   auto it = decision_nodes.begin();
        for (int i = 0; i < m; ++i) {
            //tmp :[bi] tmp1 :[1-bi]
            NTL::ZZ tmp = NTL::MulMod(NTL::PowerMod(blind_cmp_result[i], NTL::ZZ(1 - 2 * dec_d[i]), modulus * modulus),
                                      enc_dec_d[i], modulus * modulus);
            NTL::ZZ tmp1 = NTL::MulMod(one, NTL::PowerMod(tmp, NTL::ZZ(-1), modulus * modulus), modulus * modulus);
            cmp_result.push_back(tmp);
            edge_cost[it->first].push_back(tmp1);
            edge_cost[it->first].push_back(tmp);
            ++it;
        }
    }
    else
    {   auto it = enc_decision_nodes.begin();
        for (int i = 0; i < m; ++i) {
            //tmp :[bi] tmp1 :[1-bi]
            NTL::ZZ tmp = NTL::MulMod(NTL::PowerMod(blind_cmp_result[i], NTL::ZZ(1 - 2 * dec_d[i]), modulus * modulus),
                                      enc_dec_d[i], modulus * modulus);
            NTL::ZZ tmp1 = NTL::MulMod(one, NTL::PowerMod(tmp, NTL::ZZ(-1), modulus * modulus), modulus * modulus);
            cmp_result.push_back(tmp);
            edge_cost[it->first].push_back(tmp1);
            edge_cost[it->first].push_back(tmp);
            ++it;
        }
    }
    cout<<"get all the edge cost time is "<<time_from(t1)<<"us"<<endl;
    //得到path_cost
    auto t2=clock_start();
    for (auto & leaf_node : leaf_nodes) {
        int leaf_index = leaf_node.first;
        int node_index = leaf_node.second.begin()->first;
        int direction = leaf_node.second.begin()->second;
        NTL::ZZ tmp = edge_cost[node_index][direction];
        for (auto bbeg = leaf_node.second.begin() + 1; bbeg != leaf_node.second.end(); ++bbeg) {
            tmp = NTL::MulMod(tmp, edge_cost[bbeg->first][bbeg->second], modulus * modulus);
        }
        path_cost[leaf_index] = tmp;
    }
    cout<<"all leaf nodes path cost add time is "<<time_from(t2)<<"us"<<endl;
    auto t3=clock_start();
    int i = 0;
    for (auto & beg : path_cost) {
        NTL::ZZ blind_pj = NTL::PowerMod(beg.second, path_cost_r[i], modulus * modulus);
        NTL::ZZ tmp = NTL::MulMod(NTL::PowerMod(beg.second, leaf_value_r[i], modulus * modulus),
                                  enc_leaf_values[beg.first], modulus * modulus);
        NTL::ZZ blind_vj=NTL::MulMod(tmp,enc_tree_r,modulus*modulus);
        path_cost_and_value.emplace_back(blind_pj,blind_vj);//(pcj,vj)
        ++i;
    }

    srand((unsigned)time(NULL));
    random_shuffle(path_cost_and_value.begin(),path_cost_and_value.end());
    cout<<"vj pcj blind time is"<<time_from(t3)<<endl;
    auto t4=clock_start();

    //data packing
    int num_of_package=path_cost_and_value.size()/9+1;
    int last_package_num=path_cost_and_value.size()%9;
    NTL::ZZ expand_factor=NTL::PowerMod(NTL::ZZ(2),106,modulus);
    for(int i=0;i<num_of_package-1;++i)
    {   NTL::ZZ tmp_pj=path_cost_and_value[i*9+8].first;
        NTL::ZZ tmp_vj=path_cost_and_value[i*9+8].second;
        for(int j=7;j>=0;--j)
        {
            tmp_pj=NTL::MulMod(NTL::PowerMod(tmp_pj,expand_factor,modulus*modulus),path_cost_and_value[i*9+j].first,modulus*modulus);
            tmp_vj=NTL::MulMod(NTL::PowerMod(tmp_vj,expand_factor,modulus*modulus),path_cost_and_value[i*9+j].second,modulus*modulus);
        }
        pack_path_cost_and_value.emplace_back(tmp_pj,tmp_vj);
    }
    if(last_package_num==0)
    {
        package_information.first=num_of_package-1;
        package_information.second=last_package_num;
    }
    if(last_package_num!=0)
    {
        NTL::ZZ tmp_pj=path_cost_and_value[(num_of_package-1)*9+last_package_num-1].first;
        NTL::ZZ tmp_vj=path_cost_and_value[(num_of_package-1)*9+last_package_num-1].second;
        for(int j=last_package_num-2;j>=0;--j)
        {
            tmp_pj=NTL::MulMod(NTL::PowerMod(tmp_pj,expand_factor,modulus*modulus),path_cost_and_value[(num_of_package-1)*9+j].first,modulus*modulus);
            tmp_vj=NTL::MulMod(NTL::PowerMod(tmp_vj,expand_factor,modulus*modulus),path_cost_and_value[(num_of_package-1)*9+j].second,modulus*modulus);
        }
        pack_path_cost_and_value.emplace_back(tmp_pj,tmp_vj);
        package_information.first=num_of_package;
        package_information.second=last_package_num;
    }
    for(auto &beg:pack_path_cost_and_value)
    {
      NTL::ZZ tmp=PDO(beg.first);
      NTL::ZZ tmp1=PDO(beg.second);
      cout<<beg.first<<endl;
      cout<<tmp1<<endl;
      path_cost_and_value_C1.emplace_back(tmp,tmp1);
    }
    /*for(auto &beg:path_cost_and_value)
    {
      NTL::ZZ tmp=PDO(beg.first);
      NTL::ZZ tmp1=PDO(beg.second);
      cout<<beg.first<<endl;
      cout<<tmp1<<endl;
      path_cost_and_value_C1.emplace_back(tmp,tmp1);
    }*/
    cout<<" half decrypt vj pcj time is "<<time_from(t4)<<"us"<<endl;
}

void SP::step3_2(NTL::ZZ enc_tree_r){

}
void SP::genPathCostRAndValueR() {
    for(size_t i=0;i<leaf_nodes.size();++i)
    {
        long a=NTL::RandomLen_long(40);
        long b=NTL::RandomLen_long(40);
        path_cost_r.push_back(a);
        leaf_value_r.push_back(b);
    }
}

void SP::sendValueAndPathCost(NetIO *io) {
    auto start=clock_start();
    //long ciper_pair_size=leaf_num*2;
    long ciper_pair_size=pack_path_cost_and_value.size()*2;
    auto *C=new unsigned char[ciper_pair_size*256];
    auto *C1=new unsigned char[ciper_pair_size*256];

    for(long i=0;i<pack_path_cost_and_value.size();++i)
    {
        NTL::BytesFromZZ(C+2*i*256,pack_path_cost_and_value[i].first,256);
        NTL::BytesFromZZ(C+(2*i+1)*256,pack_path_cost_and_value[i].second,256);
        NTL::BytesFromZZ(C1+2*i*256,path_cost_and_value_C1[i].first,256);
        NTL::BytesFromZZ(C1+(2*i+1)*256,path_cost_and_value_C1[i].second,256);
    }
    io->send_data(&ciper_pair_size, sizeof(long));
    io->send_data(&package_information.second, sizeof(int));
    io->send_data(C,ciper_pair_size*256);
    io->send_data(C1,ciper_pair_size*256);
    io->flush();
    cout<<"send vj and pcj time is "<<time_from(start)<<endl;
    delete []C;
    delete []C1;
}

void SP::genEncLeafValues() {
    for(auto & leaf_value : leaf_values)
    {
        enc_leaf_values[leaf_value.first]=encrypt(NTL::ZZ(leaf_value.second));
    }
}

void SP::clearAllData() {

    ciper_pair.clear();
    blind_C.clear();
    blind_r.clear();
    matrix.clear();
    threshold.clear();
    enc_threshold.clear();
    decision_nodes.clear();
    leaf_values.clear();
    enc_leaf_values.clear();
    leaf_nodes.clear();
    blind_Z.clear();
    blind_cmp_result.clear();
    enc_ri.clear();
    enc_Z.clear();
    cmp_result.clear();
    edge_cost.clear();
    path_cost.clear();
    path_cost_r.clear();
    leaf_value_r.clear();
    path_cost_and_value.clear();
    pack_path_cost_and_value.clear();
    path_cost_and_value_C1.clear();
    offline_time.clear();
    enc_dec_d.clear();
    dec_d.clear();
    enc_decision_nodes.clear();
}


