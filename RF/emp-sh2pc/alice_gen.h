//
// Created by qhh on 2019/12/15.
//

#ifndef RF_ALICE_GEN_H
#define RF_ALICE_GEN_H


#include "emp-sh2pc/emp-sh2pc.h"
#include <NTL/ZZ.h>
#include "SP.h"

void gen_circuits(int party,int size,SemiHonestGen<NetIO> *gen,SP &sp);
void gen_circuits_2(int party,int size,SemiHonestGen<NetIO> *gen,SP &sp);
void gen_sort_circuits(int party,int size,SemiHonestGen<NetIO> *gen,SP &sp);
void init_circuits(SemiHonestGen<NetIO>  *gen);
#endif //RF_ALICE_GEN_H