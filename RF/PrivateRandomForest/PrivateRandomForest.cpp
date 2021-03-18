//
// Created by qhh on 2019/12/26.
//

#include "PrivateRandomForest.h"
#include <memory>
#include <fstream>
#include <ctime>
#include <algorithm>
#include<queue>

using namespace std;

extern const int PrivateNumOfFeatures;
extern const int PrivateNumOfTypes;
extern const int PrivateNumOfThreads;
extern const int PrivateNumOfTest;
extern const int PrivateNumOfTrees;

PrivateRandomForest::PrivateRandomForest(): result(PrivateNumOfTest,vector<int>((PrivateNumOfTrees+1),0)){
}

bool PrivateRandomForest::readTrainSample(string train_file, SP&sp) {
    ifstream train(train_file);
    if(!train.is_open()) {
        cerr << "[Error] :Failed to open file" <<train_file<<endl;
        return false;
    }
    cout<<"PrivateRandomForest reading sample ..."<<endl;
    all_samples.clear();
    //string s;
    //getline(train,s);
    double value;
    char dot;
    long id;
    int label;
    int cnt=0;
    while(true)
    {
        if(train.eof())
            break;
        train>>id;
        if(id==5) label=1;
        else label=0;
        /*
        train>>dot;
        char char_label;
        train>>char_label;
        if(char_label=='M') label =0;
        if(char_label=='B') label=1;
         */
        vector<NTL::ZZ>features;
        for(size_t i=0;i<PrivateNumOfFeatures;++i)
        {
            train>>dot>>value;
            int64_t padded_value=value*pow(2,29);
            features.push_back(sp.encrypt(NTL::ZZ(padded_value)));
        }
        //train>>dot>>label;
        cout<<"one time"<<cnt++<<endl;
        all_samples.emplace_back(label,features);
        if(train.peek()=='\n')
            train.get();
    }
    train.close();
    cout<<"PrivateRandomForest reading sample successful"<<endl;
    return true;
}
void PrivateRandomForest::build(SP&sp,SemiHonestGen<NetIO> *gen) {
    PrivateTree *tree=new PrivateTree();
    vector<pair<int,vector<NTL::ZZ>>>::iterator it=all_samples.begin();
    vector<pair<int,vector<NTL::ZZ>>>train_samples;

    random_shuffle(all_samples.begin(),all_samples.end());

    int all_sample_num=all_samples.size();
    for(int i=0;i<all_sample_num;++i)
    {
        //srand((unsigned)time(NULL));
        int index=rand()%all_sample_num;
        train_samples.push_back(all_samples[index]);
    }
    cout<<"bootstrap completed: single tree received shuffled example"<<endl;
    if(tree->readPrivateSample(train_samples,1))
    {
        tree->buildPrivateTree(sp,gen);
    }
    forest.push_back(tree);
    train_samples.clear();
}


bool PrivateRandomForest::buildTree(int num,SP&sp,SemiHonestGen<NetIO> *gen) {
    srand(2021);
    if(all_samples.empty())
    {
        cerr<<"PrivateRandomForest::buildTree [Error]: all_samples is empty"<<endl;
        return false;
    }
    for(int i=0;i<num;++i)
    {
        srand(2021+i*1000);
        cout<<"Tree "<<i+1<<":"<<endl;
        this->build(sp,gen);
    }
    return true;
}

bool PrivateRandomForest::readTestSample(string test_file,SP &sp) {
    ifstream test(test_file);
    if(!test.is_open())
    {
        cerr<<"[Error]: Failed to open file "<<test_file<<endl;
        return  false;
    }
    cout<<"reading test sample ..."<<endl;
    test_samples.clear();
    //string s;
    //getline(test,s);
    int64_t value;
    char dot;
    long id;
    int label = 0;
    while(true)
    {
        if(test.eof())
            break;
        test>>id;
        test>>dot;
        char char_label;
        test>>char_label;
        if(char_label=='M') label =0;
        if(char_label=='B') label=1;
        vector<NTL::ZZ>features;
        for(int i=0;i<PrivateNumOfFeatures;++i)
        {
            test>>dot>>value;
            int64_t padded_value=value*pow(2,19);
            features.push_back(sp.encrypt(NTL::ZZ(padded_value)));
        }
        cout<<label<<endl;
        //test>>dot>>label;
        test_samples.emplace_back(label,features);
        if(test.peek()=='\n')
            test.get();
    }
    test.close();
    cout<<"reading test sample sucessful"<<endl;
    return true;
}
/*
void PrivateRandomForest::predict(vector<double> &error) {
    if(test_samples.empty())
    {
        cerr<<"PrivateRandomForest::predict [Error]: test samples is empty"<<endl;
        return;
    }
    ofstream out("result.csv");
    if(!out.is_open())
    {
        cerr<<"[Error]: failed to open result.csv"<<endl;
        return;
    }
    cout<<"forest predicring ...."<<endl;
    out<<"real,predict";
    for(int i=0;i<forest.size();++i)
    {
        out<<",tree"<<i+1;
    }
    out<<"\n";
    int error_num=0;
    for(int i=0;i<test_samples.size();++i)
    {
        int real_label=test_samples[i].first;
        map<int,int>trees_result;
        for(size_t j=0;j<forest.size();++j)
        {
            int predict_label=forest[j]->predict(test_samples[i].second).first;
            result[i][j]=predict_label;
            ++trees_result[predict_label];
        }
        int max_type_times=-1;
        for(auto beg=trees_result.begin();beg!=trees_result.end();beg++)
        {
            if (beg->second >= max_type_times)
            {
                max_type_times=beg->second;
                result[i][forest.size()]=beg->first;
            }
        }
        int predicted_label=result[i][forest.size()];
        out<<real_label<<","<<predicted_label;
        for(size_t k=0;k<forest.size();++k)
        {
            out<<","<<result[i][k];
        }
        out<<"\n";
        if(real_label!=predicted_label)
            ++error_num;
        trees_result.clear();
    }
    out.close();
    double error_rate=error_num*1.0/test_samples.size();
    error.push_back(error_rate);
    cout<<"predicting completed"<<endl;
    cout<<"Error rate is  "<<error_rate<<endl;
};
*/
void PrivateRandomForest::writeForest(string file) {
    ofstream writeForest(file);
    if(!writeForest.is_open())
    {
        cerr<<"[Error] faile to open file"<<file<<endl;
    }
    cout<<"begin write"<<endl;
    writeForest<<"PrivateRandomForest: "<<forest.size()<<" Trees"<<"\n";
    for(size_t i=0;i<forest.size();++i)
    {
        writeForest<<"Tree "<<i+1<<":"<<"\n";
        writeForest<<"decision node:"<<"\n";
        map<int,pair<int,NTL::ZZ>>decision_nodes=forest[i]->getDecisionNodes();
        map<int,vector<pair<int,int>>>leaf_nodes=forest[i]->getLeafNodes().first;
        map<int,int>leaf_values=forest[i]->getLeafNodes().second;
        for(auto beg=decision_nodes.begin();beg!=decision_nodes.end();++beg)
        {
            writeForest<<beg->first<<":fea"<<beg->second.first<<" > "<<beg->second.second<<endl;
        }
        writeForest<<"leaf node:"<<"\n";
        for(auto beg=leaf_nodes.begin();beg!=leaf_nodes.end();++beg)
        {
            writeForest<<beg->first<<"("<<leaf_values[beg->first]<<"):path node (";
            for(auto bbeg=beg->second.begin();bbeg!=beg->second.end();++bbeg)
            {
                writeForest<<bbeg->first<<"("<<bbeg->second<<")"<<"<--";
            }
            writeForest<<") ";
            writeForest<<endl;
        }
        writeForest<<"\n";
    }
    writeForest.close();
}

void PrivateRandomForest::writeForestModel(string file) {
    ofstream writeForest(file);
    if(!writeForest.is_open())
    {
        cerr<<"[Error] faile to open file"<<file<<endl;
    }
    cout<<"begin write"<<endl;
    writeForest<<forest.size()<<endl;
    for(size_t i=0;i<forest.size();++i)
    {
        map<int,pair<int,NTL::ZZ>>decision_nodes=forest[i]->getDecisionNodes();
        map<int,vector<pair<int,int>>>leaf_nodes=forest[i]->getLeafNodes().first;
        map<int,int>leaf_values=forest[i]->getLeafNodes().second;
        writeForest<<decision_nodes.size()<<endl;
        for(auto beg=decision_nodes.begin();beg!=decision_nodes.end();++beg)
        {
            writeForest<<beg->first<<":"<<beg->second.first<<">"<<beg->second.second<<endl;
        }
        writeForest<<leaf_nodes.size()<<endl;
        for(auto beg=leaf_nodes.begin();beg!=leaf_nodes.end();++beg)
        {
            writeForest<<beg->first<<"("<<leaf_values[beg->first]<<"):";
            for(auto bbeg=beg->second.begin();bbeg!=beg->second.end();++bbeg)
            {
                writeForest<<bbeg->first<<"("<<bbeg->second<<")";
            }
            writeForest<<endl;
        }
        writeForest<<"\n";
    }
    writeForest.close();
}

PrivateRandomForest::~PrivateRandomForest() {
    for(size_t i=0;i<forest.size();++i)
    {
        if(forest[i]!= nullptr)
            delete(forest[i]);
    }
    //cout<<"delete forest"<<endl;
}
