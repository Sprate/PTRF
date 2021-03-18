//
// Created by Administrator on 2019/11/25.
//

#include "Paillier.h"
#include <iostream>
using namespace std;
using namespace NTL;
NTL::ZZ generateCoprimeNumber(const NTL::ZZ& n) {
    NTL::ZZ ret;
    while (true) {
        ret = RandomBnd(n);
        if (NTL::GCD(ret, n) == 1) { return ret; }
    }
}

Paillier::Paillier() {
    /* Length in bits. */
}

Paillier::Paillier(const NTL::ZZ& modulus, const NTL::ZZ& lambda) {
    this->modulus = modulus;
    generator = this->modulus + 1;
    this->lambda = lambda;
    //lambdaInverse = NTL::InvMod(this->lambda, this->modulus);
}

void Paillier::GenPrimePair(NTL::ZZ& p, NTL::ZZ& q,
                            long keyLength) {
    while (true) {
        long err = 80;
        p = NTL::GenPrime_ZZ(keyLength, err);
        q = NTL::GenPrime_ZZ(keyLength, err);
        while (p == q) {
            q = NTL::GenPrime_ZZ(keyLength, err);
            cout<<"123"<<endl;
        }
        NTL::ZZ n = p * q;
        NTL::ZZ phi = (p - 1) * (q - 1);
        if (NTL::GCD(n, phi) == 1) return;
    }
}

NTL::ZZ Paillier::encrypt(const NTL::ZZ& message) {
    clock_t start,end;
    start=clock();
    NTL::ZZ random = generateCoprimeNumber(modulus);
    /*
    NTL::ZZ ciphertext =
            NTL::PowerMod(generator, message, modulus * modulus) *
            NTL::PowerMod(random, modulus, modulus * modulus);
    */
    NTL::ZZ ciphertext=(NTL::ZZ(1)+message*modulus)*NTL::PowerMod(random, modulus, modulus * modulus);
    end=clock();
    cout<<"encrypt once run time is "<<(end-start)*1000000/CLOCKS_PER_SEC<<" us"<<endl;
    return ciphertext % (modulus * modulus);
}

NTL::ZZ Paillier::encrypt(const NTL::ZZ& message, const NTL::ZZ& random) {
    NTL::ZZ ciphertext =
            NTL::PowerMod(generator, message, modulus * modulus) *
            NTL::PowerMod(random, modulus, modulus * modulus);
    return ciphertext % (modulus * modulus);
}


NTL::ZZ Paillier::decrypt(const NTL::ZZ& ciphertext) {
    /* NOTE: NTL::PowerMod will fail if the first input is too large
     * (which I assume means larger than modulus).
     */
    clock_t start,end;
    start=clock();
    NTL::ZZ deMasked = NTL::PowerMod(
            ciphertext, lambda, modulus * modulus);
    NTL::ZZ power = L_function(deMasked);
    end=clock();
    cout<<"decrypt once run time is "<<(end-start)*1000000/CLOCKS_PER_SEC<<" us"<<endl;
    return (power * lambdaInverse) % modulus;
}

void Paillier::KeyGen() {
    long keyLength = 512;
    NTL::ZZ p, q;
    GenPrimePair(p, q, keyLength);
    cout<<"p= : "<<p<<"\n"<<"p length:"<<NumBits(p)<<endl;
    cout<<"q= : "<<q<<"\n"<<"q length:"<<NumBits(q)<<endl;
    modulus = p * q;
    generator = modulus + 1;
    cout<<"n= : "<<modulus<<"\n"<<"n length:"<<NumBits(modulus)<<endl;
    NTL::ZZ phi = (p - 1) * (q - 1);
    // LCM(p, q) = p * q / GCD(p, q);
    lambda = phi / NTL::GCD(p - 1, q - 1);
    lambdaInverse = NTL::InvMod(lambda, modulus);
    NTL::ZZ s;
    s=NTL::MulMod(lambda,NTL::InvMod(lambda,modulus*modulus),lambda*modulus*modulus);
    long SLength=NumBits(s);
    lambda1=NTL::RandomLen_ZZ(SLength)%s;
    lambda2=s-lambda1;
    cout<<"lambda"<<"\n"<<lambda<<endl;
    cout<<"s length:"<<SLength<<endl;
    cout<<"lambda1"<<"\n"<<lambda1<<endl;
    cout<<"lambda2"<<"\n"<<lambda2<<endl;
    cout<<"lambdaInverse"<<"\n"<<lambdaInverse<<endl;
}



