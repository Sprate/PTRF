#ifndef SEMIHONEST_H__
#define SEMIHONEST_H__
#include "emp-sh2pc/semihonest_gen.h"
#include "emp-sh2pc/semihonest_eva.h"

namespace emp {
template<typename IO>
inline void setup_semi_honest(IO* io, int party ,SemiHonestGen<IO>*&semiHonestGen,SemiHonestEva<IO>*&semiHonestEva ) {
	if(party == ALICE) {
		HalfGateGen<IO,RTCktOpt ::off> * t = new HalfGateGen<IO,RTCktOpt ::off>(io);
		CircuitExecution::circ_exec = t;
        semiHonestGen= new SemiHonestGen<IO>(io, t);
		ProtocolExecution::prot_exec = semiHonestGen;
	} else {
		HalfGateEva<IO,off> * t = new HalfGateEva<IO,off>(io);
		CircuitExecution::circ_exec = t;
        semiHonestEva=new SemiHonestEva<IO>(io, t);
		ProtocolExecution::prot_exec = semiHonestEva;
	}
}
}
#endif
