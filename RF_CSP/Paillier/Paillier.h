//
// Created by Administrator on 2019/11/25.
//

#ifndef RF_PAILLIER_H
#define RF_PAILLIER_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_pXFactoring.h>

class Paillier {
public:
    Paillier();
    Paillier(const NTL::ZZ& modulus, const NTL::ZZ& lambda);
    NTL::ZZ encrypt(const NTL::ZZ& message);
    NTL::ZZ encrypt(const NTL::ZZ& message, const NTL::ZZ& random);
    NTL::ZZ decrypt(const NTL::ZZ& ciphertext);

    void KeyGen();
    NTL::ZZ modulus;
    NTL::ZZ generator;
    NTL::ZZ L_function(const NTL::ZZ& n) { return (n - 1) / modulus; }
    NTL::ZZ GetLambda1(){return lambda1;}
    NTL::ZZ GetLambda2(){ return lambda2;}
private:
    /* modulus = pq, where p and q are primes */
    NTL::ZZ lambda;
    NTL::ZZ lambdaInverse;
    NTL::ZZ lambda1;
    NTL::ZZ lambda2;

    void GenPrimePair(NTL::ZZ& p, NTL::ZZ& q, long keyLength);
};

#endif //RF_PAILLIER_H
