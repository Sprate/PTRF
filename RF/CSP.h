//
// Created by Administrator on 2019/11/26.
//

#ifndef RF_CSP_H
#define RF_CSP_H

#include "Paillier/Paillier.h"
#include "RandomForest/RandomForest.h"
class CSP:public Paillier {
public:
    NTL::ZZ PDT(const NTL::ZZ &cipertext,const NTL::ZZ &C1);
    explicit CSP(Paillier &p);
    void Step2(const vector<pair<NTL::ZZ,NTL::ZZ>>  &rec_ciper_pair);
    void SetLambda2(NTL::ZZ &a)
    {
        this->lambda2=a;
    }
private:
    NTL::ZZ lambda2;
    vector<pair<NTL::ZZ,NTL::ZZ>> rec_ciper_pair;
    vector<long>blind_plain;

};


#endif //RF_CSP_H
