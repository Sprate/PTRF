//
// Created by qhh on 2019/12/26.
//

#include "PrivateTree.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <map>
#include <set>
#include <queue>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <ctime>

using namespace std;
extern const int PrivateNumOfTypes;
extern const int PrivateNumOfFeatures;
extern const int PrivateMaxDepth;
extern const int PrivateMaxFeatures;
PrivateTree::PrivateTree() {
    root=new PrivateNode();
}
//将数据读取存放到节点node中的数据，统计数据条数
bool PrivateTree::readPrivateSample(vector<pair<int, vector<NTL::ZZ>>> &all_samples,int Coe) {
    if(all_samples.empty())
    {
        cerr<<"[Error]:all sample is empty"<<endl;
    }
    if(root == nullptr)
        root=new PrivateNode();
    cout<<all_samples.size()<<endl;
    //srand((unsigned)time(NULL));
    //random_shuffle(all_samples.begin(),all_samples.end());//随机打乱
    auto p=all_samples.size()*(1-Coe);

    for(auto beg=all_samples.begin(),end=all_samples.end();beg!=(end-p);++beg)
    {
        root->private_samples.push_back(*beg);
        cout<<beg->first;
        //int type=(beg->first)-1;多分类 123
        int type=beg->first;//二分类 0 1
        (root->num[type])++;
        //int type=beg->first;
        //root->num[type-1]++;
    }
    cout<<endl;
    root->sample_num=root->private_samples.size();
    cout<<" single PrivateTree reading sample completed"<<endl;
    return true;
}

bool PrivateTree::buildPrivateTree(SP&sp,SemiHonestGen<NetIO> *gen) {
    if(root== nullptr)
        return false;
    cout<<"building PrivateTree..."<<endl;
    double is_same_coe=0.90;
    queue<PrivateNode*>node_queue;//辅助建树
    node_queue.push(root);
    int splite_times=0;//
    int tree_depth=0;
    while(!node_queue.empty())
    {
        PrivateNode *current=node_queue.front();
        node_queue.pop();
        if(current== nullptr)
            continue;
        /*
        for(size_t i=0;i<PrivateNumOfTypes;i++)
        {
            if((current->num[i]*1.0/current->sample_num)>is_same_coe)//90%样本相同限制
                continue;
        }
         */
        //if(current->depth>=MaxDepth)//深度限制
        // continue;
        if(current->sample_num<3)//单个节点样本数量限制
            continue;
        pair<PrivateNode*,PrivateNode*>child=this->selectBestSplite_2(current,sp,gen);
        current->left=child.first;
        current->right=child.second;
        tree_depth=tree_depth>current->depth?tree_depth:current->depth;
        node_queue.push(child.first);
        node_queue.push(child.second);
        if(child.first!= nullptr||child.second!= nullptr)
        {
            ++splite_times;
        }

    }
    cout<<"building a single PrivateTree completed"<<endl;
    cout<<"splite "<<splite_times<<"times"<<endl;
    cout<<"this PrivateTree depth is "<<tree_depth<<endl;
    return true;
}
pair<int,double> getType(PrivateNode *node)//计算节点当前的类型;从1开始
{
    if(node== nullptr)
        return make_pair(-1,-1);
    int max_value=-1000,index=0;
    for(int i=0;i<PrivateNumOfTypes;++i)
    {
        if(max_value<=node->num[i])

        {
            max_value=node->num[i];
            index=i;
        }
    }
    double prob=node->num[index]*1.0/node->sample_num;
    //cout<<"node index : "<<node->index<<endl;
    return make_pair(index,prob);
}
//从特征中随机选取一些属性
vector<int> PrivateTree::randomSelect(){
    vector<int> v;
    for(vector<int>::size_type i=0;i<PrivateNumOfFeatures;++i)
    {
        v.push_back(i);
    }
    random_shuffle(v.begin(),v.end());
    v.erase(v.begin()+PrivateMaxFeatures,v.end());
    cout<<"selected features: "<<endl;
    for(const auto a:v)
    {
        cout<<a<<ends;
    }
    cout<<endl;
    return v;
}
pair<PrivateNode *,PrivateNode*> PrivateTree::selectBestSplite(PrivateNode *current,SP&sp,SemiHonestGen<NetIO> *gen) {
    if(current== nullptr)
        return pair<PrivateNode*,PrivateNode*>(nullptr, nullptr);

    vector<pair<int,vector<NTL::ZZ>>>node_samples=current->private_samples;//当前节点所有的样本[(label,[28个double特征值]),(label,[28个double特征值])]

    double sample_gini=1;

    for(auto it=current->num.begin();it!=current->num.end();++it)
    {
        sample_gini-=((*it)*1.0/current->sample_num)*((*it)*1.0/current->sample_num);
    }
    if(sample_gini<0.001)
        return pair<PrivateNode*,PrivateNode*>(nullptr, nullptr);

    vector<int> selected_features=this->randomSelect();//选择出用来分裂的的属性
    map<int,vector<pair<NTL::ZZ,pair<int,int>>>>selected_feature_values;//选择出来的每个候选属性在样本中取到的值 {{feature1,{1,2,4,5}, {feature2,{1,5}} }
    int tmp_index=0;
    for(auto & node_sample : node_samples)
    {
        for(int feature_index : selected_features)
        {
            //特征索引 25,13等
            selected_feature_values[feature_index].push_back(make_pair(node_sample.second[feature_index],make_pair(node_sample.first,tmp_index)));
        }
        ++tmp_index;
    }//将所有样本根据所选出来的特征 得到一个样本值的集合
    tmp_index=0;
    //gen random r to blind node sample  feature value
    int node_sample_num=node_samples.size();
    map<int,vector<pair<int,int>>> all_sort_index_label;
    map<int,pair<int,double>>Gini;

    for(auto &tmp_sample:selected_feature_values)//five times
    {//
        int begin=12345;
        gen->io->send_data(&begin, sizeof(int));
        vector<NTL::ZZ> blind_r;
        vector<NTL::ZZ>enc_blind_r;
        vector<pair<pair<NTL::ZZ,NTL::ZZ>,pair<int,int>>>ciper_pair;
        for(int j=0;j<node_sample_num;++j)
        {
            NTL::ZZ tmp=NTL::RandomLen_ZZ(106);
            NTL::ZZ enc_tmp=sp.encrypt(NTL::ZZ(tmp));
            blind_r.push_back(tmp);
            enc_blind_r.push_back(enc_tmp);
        }
        int tmp_count=0;
        for(auto &sample :tmp_sample.second)
        {
            NTL::ZZ blind_ciper=NTL::MulMod(sample.first,enc_blind_r[tmp_count],sp.modulus*sp.modulus);
            NTL::ZZ blind_ciper_C1=sp.PDO(blind_ciper);
            tmp_count++;
            ciper_pair.emplace_back(make_pair(blind_ciper,blind_ciper_C1),sample.second);
        }

        long ciper_pair_size=ciper_pair.size();
        auto *C=new unsigned char[ciper_pair_size*256];
        auto *C1=new unsigned char[ciper_pair_size*256];
        int *label_index=new int[ciper_pair_size*2];

        for(long i=0;i<ciper_pair_size;++i)
        {
            NTL::BytesFromZZ(C+i*256,ciper_pair[i].first.first,256);
            NTL::BytesFromZZ(C1+i*256,ciper_pair[i].first.second,256);
            label_index[2*i]=ciper_pair[i].second.first;
            label_index[2*i+1]=ciper_pair[i].second.second;
        }
        gen->io->send_data(&ciper_pair_size, sizeof(long));
        //gen->io->send_data(&package_information.second, sizeof(int));
        gen->io->send_data(C,ciper_pair_size*256);
        gen->io->send_data(C1,ciper_pair_size*256);
        gen->io->send_data(label_index,ciper_pair_size*2* sizeof(int));
        delete []C;
        delete []C1;
        delete []label_index;
        gen->io->flush();

        //gen sort ciruits
        int size=node_sample_num;
        auto start=clock_start();
        uint64_t bytes=gen->io->counter;
        int feature_bit=64;
        cout<<"before gen_circuits send bytes"<<bytes<<endl;
        Batcher batcher1,batcher2,batcher3;
        Integer*A=new Integer[size];
        Integer*B=new Integer[size];
        block *input_label0_alice=new block[size*feature_bit+size*64];
        block *input_label0_bob=new block[size*feature_bit];

        bool *di=new bool[size*feature_bit];
        bool *send_di=new bool [size*64];
        //PRG prg;
        gen->shared_prg.random_block(input_label0_bob,size*feature_bit);
        //prg.random_block(input_label0_bob,size*32);
        gen->shared_prg.random_block(input_label0_alice,size*feature_bit+size*64);

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
            batcher3.add<Integer>(64,0);
        }
        batcher1.make_semi_honest(BOB);
        batcher2.make_semi_honest(ALICE);
        batcher3.make_semi_honest(ALICE);//给label
        gen->input_label_alice=gen->input_label_alice-size*feature_bit-size*64;
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
            for(int j=0;j<64;++j)
            {
                send_di[i*64+j]=getLSB(B[i][j].bit);
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
        Integer a[size];//bob y+r
        Integer b[size];//alice r
        Integer index_sample[size];
        NTL::ZZ ring=NTL::power_ZZ(2,64);
        for(int i=0;i<size;++i)
        {
            long long blind_mod_r=NTL::to_long(blind_r[i]%ring);
            b[i]=Integer(feature_bit,blind_mod_r,ALICE);
        }
        for(long i=0;i<size;++i)
        {
            index_sample[i]=Integer(64,i,ALICE);
        }
        double t2=time_from(start);
        cout<<"real input to garbled labels time "<<t2<<endl;

        block *b_lable=new block[size*feature_bit];
        for(int i=0;i<size;++i)
        {
            memcpy(b_lable+i*feature_bit,(block*)b[i].bits,feature_bit* sizeof(block));
            //gen->io->send_block((block*)b[i].bits,32);
        }
        block *index_i_label=new block[size *64];
        for(int i=0;i<size;++i)
        {
            memcpy(index_i_label+i*64,(block*)index_sample[i].bits,64* sizeof(block));
        }
        gen->io->send_data(&size, sizeof(int));
        cout<<"size bytes :"<<gen->io->counter-bytes<<endl;

        gen->io->send_block(b_lable,size*feature_bit);
        gen->io->send_block(index_i_label,size*64);

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
        //gen->ot->setup_send();
        // cout<<"bast ot "<<time_from(start)<<endl;
        gen->ot->send_impl(input_label0_bob,input_label1_bob,size*feature_bit);
        cout<<"ot bytes "<<gen->io->counter-bytes<<endl;
        cout<<" ot time "<<time_from(start)<<endl;
        //gen->io->send_data(di,size*feature_bit);
        gen->io->send_data(send_di,size*64* sizeof(bool));
        for(int i=0;i<size*64;++i) cout<<send_di[i]<<ends;
        cout<<endl;
        cout<<gen->io->counter-bytes<<endl;
        gen->io->flush();
        cout<<"all time"<<time_from(start)<<endl;

        /*sp.recvCmpResult(gen->io);
        gen->io->flush();
        cout<<"recv cmp result time is "<<time_from(start)<<"us"<<endl;*/
        delete[] A;
        delete[] input_label0_alice;
        delete[] input_label0_bob;
        delete[] input_label1_bob;
        delete[] tables;

        int64_t *sort_result=new int64_t [size];
        gen->io->recv_data(sort_result,size* sizeof(int64_t));
        gen->io->flush();
        for(int i=0;i<size;++i) cout<<sort_result[i]<<ends;
        init_circuits(gen);
        vector<pair<int,pair<int,NTL::ZZ>>>sort_feature_value;//(index,(label ,ciper))
        vector<pair<int,int>>sort_index_label;
        for(int i=0;i<size;++i)
        {
            int tt_index=ciper_pair[sort_result[i]].second.second;
            int tt_label=ciper_pair[sort_result[i]].second.first;
            NTL::ZZ tt_enc_fea=ciper_pair[sort_result[i]].first.first;
            sort_feature_value.emplace_back(tt_index,make_pair(tt_label,tt_enc_fea));
            sort_index_label.emplace_back(tt_index,tt_label);
        }
        cout<<12<<endl;
        vector<double>tmp_gini;
        for(int splite_index=0;splite_index<size-1;++splite_index)
        {
            map<int,int>D1_types;//样本值大于该划分点值的样本 集合分类情况
            map<int,int>D2_types;//样本值小于等于该划分值的样本 集合分类情况
            int D2_num=splite_index+1;
            int D1_num=size-D2_num;
            for(int j=0;j<D2_num;++j)
            {
                int type=sort_feature_value[j].second.first;
                ++D2_types[type];
            }
            for(int j=D2_num;j<size;++j)
            {
                int type=sort_feature_value[j].second.first;
                ++D1_types[type];
            }
            double D1_gini=1;
            double D2_gini=1;
            double gini=0;
            for(auto & D1_type : D1_types)
            {
                D1_gini-=((D1_type.second)*1.0/D1_num)*((D1_type.second)*1.0/D1_num);
            }
            for(auto & D2_type : D2_types)
            {
                D2_gini-=((D2_type.second)*1.0/D2_num)*((D2_type.second)*1.0/D2_num);
            }
            gini=(D1_num*1.0/(D1_num+D2_num))*D1_gini+(D2_num*1.0/(D1_num+D2_num))*D2_gini;
            tmp_gini.push_back(gini);
            D1_types.clear();
            D2_types.clear();
        }
        int splite_point_index=-1;
        double min_gini_value=1000.0;
        for(int i=0;i<size-1;++i)
        {
            if(tmp_gini[i]<=min_gini_value)
            {
                splite_point_index=i;
                min_gini_value=tmp_gini[i];
            }
        }
        all_sort_index_label[tmp_sample.first]=sort_index_label;
        Gini[tmp_sample.first]=make_pair(splite_point_index,min_gini_value);
    }
    int final_splite_index=-1;
    double min_value=1000.0;
    int attr=-1;
    for(auto &gini:Gini)
    {
       if(gini.second.second<=min_value){
           min_value=gini.second.second;
           final_splite_index=gini.second.first;
           attr=gini.first;
       }
    }
    int index_low=all_sort_index_label[attr][final_splite_index].first;
    int index_high=all_sort_index_label[attr][final_splite_index+1].first;
    NTL::ZZ splite_value_low=selected_feature_values[attr][index_low].first;
    NTL::ZZ splite_value_high=selected_feature_values[attr][index_high].first;
    NTL::ZZ splite_value=NTL::MulMod(splite_value_high,splite_value_low,sp.modulus*sp.modulus);

    cout<<"final splite_index is"<<final_splite_index<<endl;
    cout<<"index_high is "<<index_high<<endl;
    cout<<"index_low is"<<index_low<<endl;
    cout<<"gini is"<<min_value<<endl;
    cout<<"attr is "<<attr<<endl;

    PrivateNode *leftchild=new PrivateNode();//左子节点是大于
    PrivateNode *rightchild=new PrivateNode();//右子节点是小于

    for(int j=0;j<final_splite_index+1;++j)
    {
        int sample_index=all_sort_index_label[attr][j].first;
        rightchild->private_samples.push_back(node_samples[sample_index]);
        int sample_label=all_sort_index_label[attr][j].second;
        ++rightchild->num[sample_label];
        ++rightchild->sample_num;
    }
    for(int j=final_splite_index+1;j<node_sample_num;++j)
    {
        int sample_index=all_sort_index_label[attr][j].first;
        int sample_label=all_sort_index_label[attr][j].second;
        leftchild->private_samples.push_back(node_samples[sample_index]);
        ++leftchild->num[sample_label];
        ++leftchild->sample_num;
    }

    current->attribute=attr;
    current->private_value=splite_value;
    leftchild->depth=(current->depth)+1;
    leftchild->index=(current->index)*2;
    rightchild->depth=(current->depth)+1;
    rightchild->index=(current->index)*2+1;
    /*if(leftchild->sample_num==0 || rightchild->sample_num==0)
    {
        return pair<Node*,Node*>(nullptr, nullptr);
    }*/
    return pair<PrivateNode*,PrivateNode*>(leftchild,rightchild);
}

pair<PrivateNode *,PrivateNode*> PrivateTree::selectBestSplite_2(PrivateNode *current,SP&sp,SemiHonestGen<NetIO> *gen) {
    if(current== nullptr)
        return pair<PrivateNode*,PrivateNode*>(nullptr, nullptr);

    vector<pair<int,vector<NTL::ZZ>>>node_samples=current->private_samples;//当前节点所有的样本[(label,[28个double特征值]),(label,[28个double特征值])]

    double sample_gini=1;

    for(auto it=current->num.begin();it!=current->num.end();++it)
    {
        sample_gini-=((*it)*1.0/current->sample_num)*((*it)*1.0/current->sample_num);
    }
    if(sample_gini<0.0000001)
        return pair<PrivateNode*,PrivateNode*>(nullptr, nullptr);

    vector<int> selected_features=this->randomSelect();//选择出用来分裂的的属性
    map<int,vector<pair<NTL::ZZ,pair<int,int>>>>selected_feature_values;//选择出来的每个候选属性在样本中取到的值 {{feature1,{1,2,4,5}, {feature2,{1,5}} }
    int tmp_index=0;
    for(auto & node_sample : node_samples)
    {
        for(int feature_index : selected_features)
        {
            //特征索引 25,13等
            selected_feature_values[feature_index].push_back(make_pair(node_sample.second[feature_index],make_pair(node_sample.first,tmp_index)));
        }
        ++tmp_index;
    }//将所有样本根据所选出来的特征 得到一个样本值的集合
    tmp_index=0;
    //gen random r to blind node sample  feature value
    int node_sample_num=node_samples.size();
    map<int,vector<pair<int,int>>> all_sort_index_label;// fea (index 32,label 0_1)
    map<int,pair<int,double>>Gini;// fea (splite index,min gini)

    for(auto &tmp_sample:selected_feature_values)//five times
    {//
        int begin=12345;
        gen->io->send_data(&begin, sizeof(int));
        vector<NTL::ZZ> blind_r;
        vector<NTL::ZZ>enc_blind_r;
        //vector<pair<pair<NTL::ZZ,NTL::ZZ>,pair<int,int>>>ciper_pair;
        for(int j=0;j<node_sample_num;++j)
        {
            NTL::ZZ tmp=NTL::RandomLen_ZZ(106);
            NTL::ZZ enc_tmp=sp.encrypt(NTL::ZZ(tmp));
            blind_r.push_back(tmp);
            enc_blind_r.push_back(enc_tmp);
        }
        int tmp_count=0;
        vector<pair<NTL::ZZ,pair<int,int>>> ciper_label_index=tmp_sample.second;
        /*
        for(auto &sample :tmp_sample.second)
        {
            NTL::ZZ blind_ciper=NTL::MulMod(sample.first,enc_blind_r[tmp_count],sp.modulus*sp.modulus);
            NTL::ZZ blind_ciper_C1=sp.PDO(blind_ciper);
            tmp_count++;
            ciper_pair.emplace_back(make_pair(blind_ciper,blind_ciper_C1),sample.second);
        }
        */

        vector<pair<NTL::ZZ,NTL::ZZ>>send_ciper;

        int num_of_package=ciper_label_index.size()/9+1;
        int last_package_num=ciper_label_index.size()%9;
        pair<int,int> package_information;
        NTL::ZZ expand_factor=NTL::PowerMod(NTL::ZZ(2),106,sp.modulus);
        for(int i=0;i<num_of_package-1;++i)
        {
            NTL::ZZ tmp=NTL::MulMod(ciper_label_index[i*9+8].first,enc_blind_r[i*9+8],sp.modulus*sp.modulus);
            for(int j=7;j>=0;--j)
            {
                NTL::ZZ t=NTL::MulMod(ciper_label_index[i*9+j].first,enc_blind_r[i*9+j],sp.modulus*sp.modulus);
                tmp=NTL::MulMod(NTL::PowerMod(tmp,expand_factor,sp.modulus*sp.modulus),t,sp.modulus*sp.modulus);
            }
            NTL::ZZ t2=sp.PDO(tmp);
            send_ciper.emplace_back(tmp,t2);
        }
        if(last_package_num!=0){
            NTL::ZZ tmp=NTL::MulMod(ciper_label_index[(num_of_package-1)*9+last_package_num-1].first,enc_blind_r[(num_of_package-1)*9+last_package_num-1],sp.modulus*sp.modulus);
            for(int i=last_package_num-2;i>=0;--i)
            {
                NTL::ZZ t=NTL::MulMod(ciper_label_index[(num_of_package-1)*9+i].first,enc_blind_r[(num_of_package-1)*9+i],sp.modulus*sp.modulus);
                tmp=NTL::MulMod(NTL::PowerMod(tmp,expand_factor,sp.modulus*sp.modulus),t,sp.modulus*sp.modulus);
            }
            NTL::ZZ t2=sp.PDO(tmp);
            send_ciper.emplace_back(tmp,t2);
            package_information.first=num_of_package;
            package_information.second=last_package_num;
        }
        if(last_package_num==0)
        {
            package_information.first=num_of_package-1;
            package_information.second=last_package_num;
        }
        long send_ciper_size=send_ciper.size();
        auto *C=new unsigned char[send_ciper_size*256];
        auto *C1=new unsigned char[send_ciper_size*256];

        for(long i=0;i<send_ciper_size;++i)
        {
            NTL::BytesFromZZ(C+i*256,send_ciper[i].first,256);
            NTL::BytesFromZZ(C1+i*256,send_ciper[i].second,256);
        }
        gen->io->send_data(&send_ciper_size, sizeof(long));
        gen->io->send_data(&package_information.second, sizeof(int));
        gen->io->send_data(C,send_ciper_size*256);
        gen->io->send_data(C1,send_ciper_size*256);
        delete []C;
        delete []C1;
        /*
        long ciper_pair_size=ciper_label_index.size();
        int *label_index=new int[ciper_pair_size*2];
        for(long i=0;i<ciper_pair_size;++i)
        {
            label_index[2*i]=ciper_label_index[i].second.first;
            label_index[2*i+1]=ciper_label_index[i].second.second;
        }
        gen->io->send_data(label_index,ciper_pair_size*2* sizeof(int));
        delete []label_index;
         */
        gen->io->flush();

        //gen sort ciruits
        int size=node_sample_num;
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
        Integer a[size];//bob y+r
        Integer b[size];//alice r
        Integer index_sample[size];
        NTL::ZZ ring=NTL::power_ZZ(2,64);
        for(int i=0;i<size;++i)
        {
            long long blind_mod_r=NTL::to_long(blind_r[i]%ring);
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
        //gen->ot->setup_send();
        // cout<<"bast ot "<<time_from(start)<<endl;
        gen->ot->send_impl(input_label0_bob,input_label1_bob,size*feature_bit);
        cout<<"ot bytes "<<gen->io->counter-bytes<<endl;
        cout<<" ot time "<<time_from(start)<<endl;
        //gen->io->send_data(di,size*feature_bit);
        gen->io->send_data(send_di,size*index_bit* sizeof(bool));
        for(int i=0;i<size*index_bit;++i) cout<<send_di[i]<<ends;
        cout<<endl;
        cout<<gen->io->counter-bytes<<endl;
        gen->io->flush();
        cout<<"all time"<<time_from(start)<<endl;

        /*sp.recvCmpResult(gen->io);
        gen->io->flush();
        cout<<"recv cmp result time is "<<time_from(start)<<"us"<<endl;*/
        delete[] A;
        delete[] input_label0_alice;
        delete[] input_label0_bob;
        delete[] input_label1_bob;
        delete[] tables;

        int64_t *sort_result=new int64_t [size];
        gen->io->recv_data(sort_result,size* sizeof(int64_t));
        gen->io->flush();
        for(int i=0;i<size;++i) cout<<sort_result[i]<<ends;
        init_circuits(gen);
        //vector<pair<int,pair<int,NTL::ZZ>>>sort_feature_value;//(index,(label ,ciper))
        vector<pair<int,int>>sort_index_label;
        for(int i=0;i<size;++i)
        {
            int tt_index=ciper_label_index[sort_result[i]].second.second;
            int tt_label=ciper_label_index[sort_result[i]].second.first;
            NTL::ZZ tt_enc_fea=ciper_label_index[sort_result[i]].first;
            sort_index_label.emplace_back(tt_index,tt_label);
        }
        cout<<12<<endl;
        vector<double>tmp_gini;
        for(int splite_index=0;splite_index<size-1;++splite_index)
        {
            map<int,int>D1_types;//样本值大于该划分点值的样本 集合分类情况
            map<int,int>D2_types;//样本值小于等于该划分值的样本 集合分类情况
            int D2_num=splite_index+1;
            int D1_num=size-D2_num;
            for(int j=0;j<D2_num;++j)
            {
                int type=sort_index_label[j].second;
                ++D2_types[type];
            }
            for(int j=D2_num;j<size;++j)
            {
                int type=sort_index_label[j].second;
                ++D1_types[type];
            }
            double D1_gini=1;
            double D2_gini=1;
            double gini=0;
            for(auto & D1_type : D1_types)
            {
                D1_gini-=((D1_type.second)*1.0/D1_num)*((D1_type.second)*1.0/D1_num);
            }
            for(auto & D2_type : D2_types)
            {
                D2_gini-=((D2_type.second)*1.0/D2_num)*((D2_type.second)*1.0/D2_num);
            }
            gini=(D1_num*1.0/(D1_num+D2_num))*D1_gini+(D2_num*1.0/(D1_num+D2_num))*D2_gini;
            tmp_gini.push_back(gini);
            D1_types.clear();
            D2_types.clear();
        }
        int splite_point_index=-1;
        double min_gini_value=1000.0;
        for(int i=0;i<size-1;++i)
        {
            if(tmp_gini[i]<=min_gini_value)
            {
                splite_point_index=i;
                min_gini_value=tmp_gini[i];
            }
        }
        all_sort_index_label[tmp_sample.first]=sort_index_label;
        Gini[tmp_sample.first]=make_pair(splite_point_index,min_gini_value);
    }
    int final_splite_index=-1;
    double min_value=1000.0;
    int attr=-1;
    for(auto &gini:Gini)
    {
        if(gini.second.second<=min_value){
            min_value=gini.second.second;
            final_splite_index=gini.second.first;
            attr=gini.first;
        }
    }
    int index_low=all_sort_index_label[attr][final_splite_index].first;//样本 index
    int index_high=all_sort_index_label[attr][final_splite_index+1].first;
    NTL::ZZ splite_value_low=selected_feature_values[attr][index_low].first;
    NTL::ZZ splite_value_high=selected_feature_values[attr][index_high].first;
    NTL::ZZ splite_value=NTL::MulMod(splite_value_high,splite_value_low,sp.modulus*sp.modulus);

    cout<<"final splite_index is"<<final_splite_index<<endl;
    cout<<"index_high is "<<index_high<<endl;
    cout<<"index_low is"<<index_low<<endl;
    cout<<"gini is"<<min_value<<endl;
    cout<<"attr is "<<attr<<endl;

    PrivateNode *leftchild=new PrivateNode();//左子节点是大于
    PrivateNode *rightchild=new PrivateNode();//右子节点是小于

    for(int j=0;j<final_splite_index+1;++j)
    {
        int sample_index=all_sort_index_label[attr][j].first;
        rightchild->private_samples.push_back(node_samples[sample_index]);
        int sample_label=all_sort_index_label[attr][j].second;
        ++rightchild->num[sample_label];
        ++rightchild->sample_num;
    }
    for(int j=final_splite_index+1;j<node_sample_num;++j)
    {
        int sample_index=all_sort_index_label[attr][j].first;
        int sample_label=all_sort_index_label[attr][j].second;
        leftchild->private_samples.push_back(node_samples[sample_index]);
        ++leftchild->num[sample_label];
        ++leftchild->sample_num;
    }

    current->attribute=attr;
    current->private_value=splite_value;
    leftchild->depth=(current->depth)+1;
    leftchild->index=(current->index)*2;
    rightchild->depth=(current->depth)+1;
    rightchild->index=(current->index)*2+1;
    /*if(leftchild->sample_num==0 || rightchild->sample_num==0)
    {
        return pair<Node*,Node*>(nullptr, nullptr);
    }*/
    return pair<PrivateNode*,PrivateNode*>(leftchild,rightchild);
}

PrivateTree::~PrivateTree() {
    PrivateNode *head=root;
    queue<PrivateNode *>q;
    q.push(head);
    while(!q.empty())
    {
        head=q.front();
        q.pop();
        if(head== nullptr)
            continue;
        PrivateNode *left=head->left;
        PrivateNode *right=head->right;
        delete(head);
        q.push(left);
        q.push(right);
    }
    //cout<<"delete tree"<<endl;
}

map<int, pair<int, NTL::ZZ>> PrivateTree::getDecisionNodes() {
    queue<PrivateNode *> q;
    q.push(root);
    map<int, pair<int, NTL::ZZ>>decsion_nodes;
    while (!q.empty())
    {
        PrivateNode *current=q.front();
        q.pop();
        if(current== nullptr)
            continue;
        if(current->left==nullptr &&current->right== nullptr)
            continue;
        decsion_nodes[current->index]=make_pair(current->attribute,current->private_value);
        q.push(current->left);
        q.push(current->right);
    }
    return decsion_nodes;
}

pair<map<int, vector<pair<int,int>>>,map<int,int>> PrivateTree::getLeafNodes() {
    queue<PrivateNode *> q;
    q.push(root);
    map<int, vector<pair<int,int>>>leaf_nodes;
    map<int,int>leaf_values;
    while (!q.empty())
    {
        PrivateNode *current=q.front();
        q.pop();
        if(current== nullptr)
            continue;
        if(current->left==nullptr &&current->right== nullptr)
        {
            int index=current->index;
            while(index!=1)
            {
                int direction=index%2;
                index=index/2;
                leaf_nodes[current->index].push_back(make_pair(index,direction));
            }
            leaf_values[current->index]=getType(current).first;
            continue;
        }
        q.push(current->left);
        q.push(current->right);
    }
    return make_pair(leaf_nodes,leaf_values);
}
