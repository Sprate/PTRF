//
// Created by qhh on 2019/12/17.
//

#ifndef RF_UTIL_H
#define RF_UTIL_H

#include <Paillier/Paillier.h>
#include <NTL/ZZ.h>
#include "emp-sh2pc/emp-sh2pc.h"
#include "UserBehavior/touchPoint.h"
#include<cmath>
using namespace std;
using namespace emp;
using namespace NTL;

void send_key(Paillier &auth,NetIO * io){
    long length_n= NumBytes(auth.modulus);
    long length_lambda=NumBytes(auth.GetLambda2());
    unsigned char send_modulus[length_n];
    unsigned char send_gengenerator[length_n];
    unsigned char send_lambda2[length_lambda];
    BytesFromZZ(send_modulus,auth.modulus,length_n);
    BytesFromZZ(send_gengenerator,auth.generator,length_n);
    BytesFromZZ(send_lambda2,auth.GetLambda2(),length_lambda);

    io->send_data(&length_n, sizeof(long));
    io->send_data(&length_lambda, sizeof(long));
    io->send_data(send_modulus,length_n);
    io->send_data(send_gengenerator,length_n);
    io->send_data(send_lambda2,length_lambda);

}
/*void send_ciper(SP &sp,NetIO *&io)
{
    long ciper_pair_size=sp.getCiperPair().size();
    unsigned char *C=new unsigned char[ciper_pair_size*256];
    unsigned char *C1=new unsigned char[ciper_pair_size*256];
    vector<pair<NTL::ZZ,NTL::ZZ>>ciper_pair=sp.getCiperPair();
    for(size_t i=0;i<ciper_pair_size;++i)
    {
        BytesFromZZ(C+i*256,ciper_pair[i].first,256);
        BytesFromZZ(C1+i*256,ciper_pair[i].second,256);
    }
    io->send_data(&ciper_pair_size, sizeof(long));
    io->send_data(C,ciper_pair_size*256);
    io->send_data(C1,ciper_pair_size*256);
    delete []C;
    delete []C1;
}*/
/*
void tenFoldCross(string file,int tree_num) {
    ifstream read_data(file);
    if(!read_data.is_open()) {
        cerr << "[Error] :Failed to open file" <<file<<endl;
    }
    cout<<"ten fold beginning"<<endl;
    vector<pair<int,vector<double>>>ten_fold_samples;
    //string s;
    //getline(train,s);
    double value;
    char dot;
    long id;
    int label;
    while(true)
    {
        if(read_data.eof())
            break;
        read_data>>id;
        read_data>>dot;
        char char_label;
        read_data>>char_label;
        if(char_label=='M') label =1;
        if(char_label=='B') label=2;

        vector<double>features;
        for(size_t i=0;i<NumOfFeatures;++i)
        {
            read_data>>dot>>value;
            features.push_back(value);
        }
        //train>>dot>>label;
        ten_fold_samples.emplace_back(label,features);
        if(read_data.peek()=='\n')
            read_data.get();
    }
    read_data.close();
    cout<<"ten fold cross reading sample successful"<<endl;
    cout<<ten_fold_samples.size()<<endl;
    srand((unsigned)time(NULL));
    random_shuffle(ten_fold_samples.begin(),ten_fold_samples.end());
    vector<double>error;
    for(int test_time=0;test_time<10;test_time++)
    {
        RandomForest forest;
        vector<pair<int,vector<double>>>tmp_samples=ten_fold_samples;
        vector<pair<int,vector<double>>>test_samples;
        vector<pair<int,vector<double>>>all_samples;
        if(test_time==9)
        {
            test_samples.assign(tmp_samples.begin()+test_time*57,tmp_samples.end());
            tmp_samples.erase(tmp_samples.begin()+test_time*57,tmp_samples.end());
        }
        else{
            test_samples.assign(tmp_samples.begin()+test_time*57,tmp_samples.begin()+(test_time+1)*57);
            tmp_samples.erase(tmp_samples.begin()+test_time*57,tmp_samples.begin()+(test_time+1)*57);
        }
        for(const auto &sample:tmp_samples)
        {
            all_samples.push_back(sample);
        }
        cout<<test_samples.size()<<all_samples.size()<<endl;
        forest.setAllSamples(all_samples);
        forest.setTestSamples(test_samples);
        forest.buildTree(tree_num);
        forest.predict(error);
    }
    double aveage_error=0;
    for(const auto e:error)
    {
        aveage_error+=e;
    }
    cout<<"ten folf cross error is "<<aveage_error/10<<endl;
}
*/
#endif //RF_UTIL_H
