//
// Created by Administrator on 2019/11/20.
//

#include "Tree.h"
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
extern const int NumOfTypes;
extern const int NumOfFeatures;
extern const int MaxDepth;
extern const int MaxFeatures;
Tree::Tree() {
    root=new Node();
}
//将数据读取存放到节点node中的数据，统计数据条数
bool Tree::readSample(vector<pair<int, vector<double>>> &all_samples,int Coe) {
    if(all_samples.empty())
    {
        cerr<<"[Error]:all sample is empty"<<endl;
    }
    if(root == nullptr)
        root=new Node();
    cout<<all_samples.size()<<endl;
    //srand((unsigned)time(NULL));
    //random_shuffle(all_samples.begin(),all_samples.end());//随机打乱
    auto p=all_samples.size()*(1-Coe);

    for(auto beg=all_samples.begin(),end=all_samples.end();beg!=(end-p);++beg)
    {
        root->samples.push_back(*beg);
        cout<<beg->first;
        //int type=(beg->first)-1;多分类 123
        int type=beg->first;//二分类 0 1
        (root->num[type])++;
        //int type=beg->first;
        //root->num[type-1]++;
    }
    cout<<endl;
    root->sample_num=root->samples.size();
    cout<<" single tree reading sample completed"<<endl;
    return true;
}

bool Tree::buildTree() {
    if(root== nullptr)
    return false;
    cout<<"building tree..."<<endl;
    double is_same_coe=0.90;
    queue<Node*>node_queue;//辅助建树
    node_queue.push(root);
    int splite_times=0;//
    int tree_depth=0;
    while(!node_queue.empty())
    {
        Node *current=node_queue.front();
        node_queue.pop();
        if(current== nullptr)
            continue;
        /*
        for(size_t i=0;i<NumOfTypes;i++)
        {
            if((current->num[i]*1.0/current->sample_num)>is_same_coe)//90%样本相同限制
                continue;
        }
         */
        if(current->depth>=16)//深度限制
           continue;
        if(current->sample_num<175)//单个节点样本数量限制
            continue;
        pair<Node*,Node*>child=this->selectBestSplite(current);
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
    cout<<"train num is "<<root->sample_num<<endl;
    cout<<"building a single tree completed"<<endl;
    cout<<"splite "<<splite_times<<"times"<<endl;
    cout<<"this tree depth is "<<tree_depth<<endl;
    return true;
}
pair<int,double> getType(Node *node)//计算节点当前的类型;从1开始
{
    if(node== nullptr)
        return make_pair(-1,-1);
    int max_value=-1000,index=0;
    for(int i=0;i<NumOfTypes;++i)
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
vector<int> Tree::randomSelect(){
    vector<int> v;
    for(vector<int>::size_type i=0;i<NumOfFeatures;++i)
    {
        v.push_back(i);
    }
    random_shuffle(v.begin(),v.end());
    v.erase(v.begin()+MaxFeatures,v.end());
    cout<<"selected features: "<<endl;
    for(const auto a:v)
    {
        cout<<a<<ends;
    }
    cout<<endl;
    return v;
}
pair<Node *,Node*> Tree::selectBestSplite(Node *current) {
    if(current== nullptr)
        return pair<Node*,Node*>(nullptr, nullptr);

    vector<pair<int,vector<double>>>node_samples=current->samples;//当前节点所有的样本[(label,[28个double特征值]),(label,[28个double特征值])]

    double sample_gini=1;
    /*
    map<int,int>D_type;
    for(auto it=node_samples.begin();it!=node_samples.end();++it)
    {
        int type=it->first;
        ++D_type[type];
    }
    for(auto itt=D_type.begin();itt!=D_type.end();++itt)
    {
        sample_gini-=((itt->second)*1.0/node_samples.size())*((itt->second)*1.0/node_samples.size());
    }
    D_type.clear();
    if(sample_gini<0.001)
        return pair<Node*,Node*>(nullptr, nullptr);//检测当前节点中gini是否小于指定值 若是 则迭代停止
   */
    for(auto it=current->num.begin();it!=current->num.end();++it)
    {
        sample_gini-=((*it)*1.0/current->sample_num)*((*it)*1.0/current->sample_num);
    }
    if(sample_gini<0.0001)
        return pair<Node*,Node*>(nullptr, nullptr);

    vector<int> selected_features=this->randomSelect();//选择出用来分裂的的属性
    map<int,set<double>>selected_feature_values;//选择出来的每个候选属性在样本中取到的值 {{feature1,{1,2,4,5}, {feature2,{1,5}} }
    //map<int,vector<double>>selected_feature_values;
    for(auto & node_sample : node_samples)
    {
        for(int feature_index : selected_features)
        {
            //特征索引 25,13等
            selected_feature_values[feature_index].insert(node_sample.second[feature_index]);
            //selected_feature_values[feature_index].push_back(node_sample.second[feature_index]);
            //sort(selected_feature_values[feature_index].begin(),selected_feature_values[feature_index].end());
        }
    }//将所有样本根据所选出来的特征 得到一个样本值的集合

    map<int,vector<double>>splite_point;//将连续值取中位数 作为候选划分点

    for(auto beg=selected_feature_values.begin();beg!=selected_feature_values.end();++beg)
    {
        vector<double>tmp;
        tmp.assign(beg->second.begin(),beg->second.end());
        int index=beg->first;
        for(auto tmp_beg=tmp.begin();tmp_beg!=(tmp.end()-1);++tmp_beg)
        {
            double mid_value=((*tmp_beg)+(*(tmp_beg+1)))*0.5;
            splite_point[index].push_back(mid_value);
        }
        tmp.clear();
    }//对连续值的处理 将每一特征的对应样本值的中位数做划分点

    //splite_point=selected_feature_values;
    map<pair<int,double>,double>Gini;
    map<int,vector<double>>::iterator bsp=splite_point.begin();
    for(;bsp!=splite_point.end();++bsp)
    {
        int attr=bsp->first;//特征index
        vector<double>::iterator bsp_v=bsp->second.begin();
        for(;bsp_v!=bsp->second.end();bsp_v++)
        {
            double splite_point_value=*bsp_v;
            map<int,int>D1_types;//样本值大于该划分点值的样本 集合分类情况
            map<int,int>D2_types;//样本值小于等于该划分值的样本 集合分类情况
            int D1_num=0;
            int D2_num=0;
            vector<pair<int,vector<double>>>::iterator bns=node_samples.begin();
            for(;bns!=node_samples.end();++bns)
            {
                int type=bns->first;
                if(bns->second[attr]>splite_point_value)
                {
                    ++D1_types[type];
                    ++D1_num;
                }
                else{
                    ++D2_types[type];
                    ++D2_num;
                }
            }//根据某一特征的某一划分点将样本分为D1和D2,并统计了D1的数量和D2的数量 及D1D2中分类情况
            double D1_gini=1;
            double D2_gini=1;
            double gini=0;
            for(auto beg=D1_types.begin();beg!=D1_types.end();++beg)
            {
                D1_gini-=((beg->second)*1.0/D1_num)*((beg->second)*1.0/D1_num);
            }
            for(auto beg=D2_types.begin();beg!=D2_types.end();++beg)
            {
                D2_gini-=((beg->second)*1.0/D2_num)*((beg->second)*1.0/D2_num);
            }
            gini=(D1_num*1.0/(D1_num+D2_num))*D1_gini+(D2_num*1.0/(D1_num+D2_num))*D2_gini;
            Gini[make_pair(attr,splite_point_value)]=gini;
            D1_types.clear();
            D2_types.clear();
        }
    }//得到了每一特征，每一划分点 对应的gini {(attr,splite_value):gini,,,,,;}
    int attr=0;
    double value=0;
    double min_value=1000;
    map<pair<int,double>,double>::iterator bgini=Gini.begin();
    for(;bgini!=Gini.end();++bgini)
    {
        if(bgini->second<=min_value)
        {
            min_value=bgini->second;
            attr=bgini->first.first;
            value=bgini->first.second;
        }
    }//得到gini系数最小的特征属性和划分值
    cout<<"gini is"<<min_value<<endl;
    Node *leftchild=new Node();//左子节点是大于
    Node *rightchild=new Node();//右子节点是小于

    vector<pair<int,vector<double>>>::iterator ins=node_samples.begin();
    for(;ins!=node_samples.end();++ins)
    {
        if(ins->second[attr]>value)
        {
            leftchild->samples.push_back(*ins);
            ++leftchild->num[ins->first];
            ++leftchild->sample_num;
        }
        else
        {
            rightchild->samples.push_back(*ins);
            ++rightchild->num[ins->first];
            ++rightchild->sample_num;
        }
    }
    current->attribute=attr;
    current->value=value;
    leftchild->depth=(current->depth)+1;
    leftchild->index=(current->index)*2;
    rightchild->depth=(current->depth)+1;
    rightchild->index=(current->index)*2+1;
    /*if(leftchild->sample_num==0 || rightchild->sample_num==0)
    {
        return pair<Node*,Node*>(nullptr, nullptr);
    }*/

    return pair<Node*,Node*>(leftchild,rightchild);

}
pair<int,double >Tree::predict(vector<double> features) {
    if(root== nullptr)
    {
        cerr<<"[Error]: empty tree"<<endl;
        return make_pair(-1,-1);
    }
    Node *head=root;
    while(true)
    {
        if(head->left==nullptr&&head->right== nullptr)
            return  getType(head);
        int attr=head->attribute;
        double value=head->value;
        if(features[attr]>value)
        {
            if(head->left== nullptr)
                return getType((head));
            else
            {
                //cout<<"path node :"<<head->index<<ends;
                head=head->left;
            }
        }
        else
        {
            if(head->right== nullptr)
                return getType(head);
            else
            {
                //cout<<"path node :"<<head->index<<ends;
                head=head->right;
            }
        }
    }
}

Tree::~Tree() {
    Node *head=root;
    queue<Node *>q;
    q.push(head);
    while(!q.empty())
    {
        head=q.front();
        q.pop();
        if(head== nullptr)
            continue;
        Node *left=head->left;
        Node *right=head->right;
        delete(head);
        q.push(left);
        q.push(right);
    }
    //cout<<"delete tree"<<endl;
}

map<int, pair<int, double>> Tree::getDecisionNodes() {
    queue<Node *> q;
    q.push(root);
    map<int, pair<int, double>>decsion_nodes;
    while (!q.empty())
    {
        Node *current=q.front();
        q.pop();
        if(current== nullptr)
            continue;
        if(current->left==nullptr &&current->right== nullptr)
            continue;
        decsion_nodes[current->index]=make_pair(current->attribute,current->value);
        q.push(current->left);
        q.push(current->right);
    }
    return decsion_nodes;
}

pair<map<int, vector<pair<int,int>>>,map<int,int>> Tree::getLeafNodes() {
    queue<Node *> q;
    q.push(root);
    map<int, vector<pair<int,int>>>leaf_nodes;
    map<int,int>leaf_values;
    while (!q.empty())
    {
        Node *current=q.front();
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



