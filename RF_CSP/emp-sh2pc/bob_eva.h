//
// Created by bonjour on 2019/12/17.
//

#ifndef RF_CSP_BOB_EVA_H
#define RF_CSP_BOB_EVA_H


#include "emp-sh2pc/emp-sh2pc.h"
#include <NTL/ZZ.h>
#include "semihonest.h"
#include <iostream>

#include "CSP.h"

void eva_circuit(SemiHonestEva<NetIO>*eva,CSP &csp);
void eva_circuit_plain(SemiHonestEva<NetIO>*eva,CSP &csp);
void initial_eva(SemiHonestEva<NetIO>*eva);
void eva_circuit_encryptd(SemiHonestEva<NetIO>*eva,CSP &csp);
#endif //RF_CSP_BOB_EVA_H
