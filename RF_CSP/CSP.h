//
// Created by Administrator on 2019/11/26.
//

#ifndef RF_CSP_CSP_H
#define RF_CSP_CSP_H

#include "Paillier/Paillier.h"
#include<vector>
#include "emp-sh2pc/emp-sh2pc.h"

using namespace std;
using namespace emp;
using namespace NTL;
class CSP:public Paillier {
public:
    NTL::ZZ PDT(const NTL::ZZ &cipertext,const NTL::ZZ &C1);
    explicit CSP(Paillier &p);
    CSP(){};
    void SetLambda2(NTL::ZZ z)
    {
        this->lambda2=z;
    };
    NTL::ZZ GetLambda2(){ return lambda2;}
    void Step2();
    void step4(NetIO *io);
    vector<NTL::ZZ>getBlindPlain(){return blind_plain;}
    void recvKey(NetIO * io);
    void recvCiper(NetIO* io);
    vector<pair<NTL::ZZ,NTL::ZZ>> getRecCiperPair(){ return  rec_ciper_pair;}
    long length_n=0;
    void clearCsp();
    void train_step(NetIO *io,SemiHonestEva<NetIO>*evav);
    void train_step_2(NetIO *io,SemiHonestEva<NetIO>*eva);
private:
    NTL::ZZ lambda2;
    vector<pair<NTL::ZZ,NTL::ZZ>> rec_ciper_pair;
    vector<NTL::ZZ>blind_plain;
    vector<pair<pair<NTL::ZZ,NTL::ZZ>,pair<NTL::ZZ,NTL::ZZ>>>pcj_vj;
    int last_package_num=0;
    int num_of_pakage=0;
};


#endif //RF_CSP_CSP_H
