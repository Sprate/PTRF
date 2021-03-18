//
// Created by qhh on 2019/12/26.
//

#include "PrivateNode.h"
#include <string.h>
PrivateNode::PrivateNode():num(PrivateNumOfTypes,0) {
    left=nullptr;
    right= nullptr;
    sample_num=0;
    //memset(num,0, sizeof(num));
    /*for(size_t i=0;i<NumOfTypes;++i)
    {
        num.push_back(0);
    }*/
    attribute=0;
    value=0;
    depth=0;
    index=1;
    private_value=NTL::ZZ(0);//add
}
PrivateNode::PrivateNode(const PrivateNode &other) {
    this->attribute=other.attribute;
    this->value=other.value;
    this->left = other.left;
    this->right = other.right;
    this->sample_num = other.sample_num;
    this->samples = other.samples;
    this->depth = other.depth;
    this->num=other.num;
    this->index=other.index;
    this->private_samples=other.private_samples;//add
    this->private_value=other.private_value;//add
    //for (int i = 0; i < NumOfTypes; i++)
    // this->num[i] = other.num[i];
}