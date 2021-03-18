//
// Created by Administrator on 2019/11/26.
//

#include "CSP.h"

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

void CSP::Step2(const vector<pair<NTL::ZZ,NTL::ZZ>> &rec_ciper_pair ) {
    //vector<pair<NTL::ZZ,NTL::ZZ>> rec_ciper_pair;
    clock_t start,end;
    start=clock();
    for(vector<pair<NTL::ZZ,NTL::ZZ>>::size_type i=0;i<rec_ciper_pair.size();++i)
    {
       NTL::ZZ tmp;
       tmp=PDT(rec_ciper_pair[i].first,rec_ciper_pair[i].second);
       cout<<"y+r is"<<tmp<<endl;
       blind_plain.push_back(NTL::to_long(tmp));
    }
    end=clock();
    cout<<"step2 run time is "<<(end-start)*1000000/CLOCKS_PER_SEC<<" us"<<endl;
}
