#ifndef SEMIHONEST_GEN_H__
#define SEMIHONEST_GEN_H__
#include <emp-tool/emp-tool.h>
#include <emp-ot/emp-ot.h>
#include <iostream>
#include <vector>

namespace emp {
template<typename IO>
class SemiHonestGen: public ProtocolExecution {
public:
	IO* io;
	SHOTExtension<IO> * ot;
	PRG prg, shared_prg;
	HalfGateGen<IO,off> * gc;
    block *input_label_alice= nullptr;
    block *input_label_bob= nullptr;
    block *input_label_index= nullptr;
    bool *di= nullptr;
	SemiHonestGen(IO* io, HalfGateGen<IO,RTCktOpt ::off>* gc): ProtocolExecution(ALICE) {
		this->io = io;
		ot = new SHOTExtension<IO>(io);
		this->gc = gc;	
		block seed;prg.random_block(&seed, 1);
		io->send_block(&seed, 1);
		shared_prg.reseed(&seed);

	}
	~SemiHonestGen() {
		delete ot;
	}

	void feed(block * label, int party, const bool* b, int length) {
		if(party == ALICE) {
			for (int i = 0; i < length; ++i) {
			    label[i]=input_label_alice[i];
				if(b[i])
					label[i] = xorBlocks(label[i], gc->delta);
			}
			input_label_alice+=length;
		}
		if(party==BOB)
        {
		    for(int i=0;i<length;++i)
            {
		        label[i]=input_label_bob[i];
		        if (b[i])
		            label[i]=xorBlocks((label[i]),gc->delta);
            }
		    input_label_bob+=length;
        }
		//else {

			//ot->send_cot(label, gc->delta, length);
		//}
	}

	/*void reveal(bool* b, int party, const block * label, int length) {
		if (party == XOR) {
			for (int i = 0; i < length; ++i) {
				if(isOne(&label[i]) or isZero(&label[i]))
					b[i] = false;
				else 
					b[i] = getLSB(label[i]);
			}
			return;
		}
		for (int i = 0; i < length; ++i) {
			if(isOne(&label[i]))
				b[i] = true;
			else if (isZero(&label[i]))
				b[i] = false;
			else {
				bool lsb = getLSB(label[i]);
				if (party == BOB or party == PUBLIC) {
					io->send_data(&lsb, 1);
					b[i] = false;
				} else if(party == ALICE) {
					bool tmp;
					io->recv_data(&tmp, 1);
					b[i] = (tmp != lsb);
				}
			}
		}
		if(party == PUBLIC)
			io->recv_data(b, length);
	}*/
    void reveal(bool* b, int party, const block * label, int length){
        for (int i = 0; i < length; ++i) {
            if(isOne(&label[i]))
                b[i] = true;
            else if (isZero(&label[i]))
                b[i] = false;
            else {
                bool lsb = getLSB(label[i]);
                if (party == BOB or party == PUBLIC) {
                    io->send_data(&lsb, 1);
                    b[i] = false;
                } else if(party == ALICE) {
                    b[i] = (di[i] != lsb);
                }
            }
        }

    }
};
}
#endif //SEMIHONEST_GEN_H__