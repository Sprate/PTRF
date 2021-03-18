//
// Created by qhh on 2019/12/17.
//

#ifndef RF_READFOREST_H
#define RF_READFOREST_H

#include "RandomForest.h"
#include <fstream>
bool readForest(string file,vector<map<int,pair<int, int64_t >>> &decision_nodes,vector<map<int,vector<pair<int,int>>>> &leaf_nodes,vector<map<int,int>>&leaf_values)
{
    ifstream readForest(file);
    if(!readForest.is_open())
    {
        cerr<<"[Error]: Failed to open file "<<file<<endl;
        return false;
    }
    cout<<"read Forest model... "<<endl;
    int forest_size;
    int decision_nodes_size;
    int leaf_nodes_size;
    readForest>>forest_size;
    readForest.get();
    for(int i=0;i<forest_size;++i)
    {   map<int,pair<int,int64_t >> tmp;
        readForest>>decision_nodes_size;
        readForest.get();
        for(int j=0;j<decision_nodes_size;++j)
        {   char dot;
            int index;
            int attr;
            double value;
            readForest>>index>>dot>>attr>>dot>>value;
            int64_t padded_value=value*pow(2,20);
            //uint64_t shifted_value= padded_value +(uint64_t)pow(2,63);
            tmp[index]=make_pair(attr,padded_value);
            cout<<index<<ends;
            cout<<attr<<ends;
            cout<<value<<endl;
            //cout<<shifted_value<<endl;
            readForest.get();
        }
        decision_nodes.push_back(tmp);
        tmp.clear();

        readForest>>leaf_nodes_size;
        readForest.get();
        map<int,vector<pair<int,int>>>tmp2;
        map<int,int>tmp3;
        for(int j=0;j<leaf_nodes_size;++j)
        {
            char dot;
            int index;
            int child_index;
            int direction;
            int value;
            readForest>>index>>dot>>value>>dot>>dot;
            tmp3[index]=value;
            while(readForest.peek()!='\n')
            {
                readForest>>child_index>>dot>>direction>>dot;
                tmp2[index].push_back(make_pair(child_index,direction));
                cout<<index<<ends;
                cout<<child_index<<ends;
                cout<<direction<<endl;
            }
            readForest.get();
        }
        leaf_nodes.push_back(tmp2);
        leaf_values.push_back(tmp3);
        tmp2.clear();
        tmp3.clear();
        readForest.get();
    }
    cout<<"reading model completed"<<endl;
    readForest.close();
    return true;
}


#endif //RF_READFOREST_H
