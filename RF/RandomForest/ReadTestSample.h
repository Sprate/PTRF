//
// Created by qhh on 2019/12/23.
//

#ifndef RF_READTESTSAMPLE_H
#define RF_READTESTSAMPLE_H
#include <fstream>
#include "RandomForest.h"
bool readSample(string test_file,vector<pair<int,vector<int64_t >>>&test_samples) {
    ifstream test(test_file);
    if(!test.is_open())
    {
        cerr<<"[Error]: Failed to open file "<<test_file<<endl;
        return  false;
    }
    cout<<"reading test sample ..."<<endl;
    //string s;
    //getline(test,s);
    double value;
    int64_t pad_value;
    char dot;
    long id;
    int label=0;
    while(true)
    {
        //test>>id;
        if(test.eof())
            break;
        //test>>id;
        /*
        if(id==5) label=1;
        else label=0;
         */
        /*
        test>>dot;
        char char_label;
        test>>char_label;
        if(char_label=='M') label =0;
        if(char_label=='B') label=1;
         */
        vector<int64_t >features;
        test>>value;
        pad_value=value*pow(2,20);
        features.push_back(pad_value);
        for(int i=0;i<NumOfFeatures-1;++i)
        {
            test>>dot>>value;
            pad_value=value*pow(2,20);
            //uint64_t shifted_value=pad_value+(uint64_t) pow(2,63);
            features.push_back(pad_value);
        }
        test>>dot>>label;
        cout<<label<<endl;
        //test>>dot>>label;
        test_samples.emplace_back(label,features);
        if(test.peek()=='\n')
            test.get();
    }
    test.close();
    cout<<"reading test sample sucessful"<<endl;
    return true;
}
#endif //RF_READTESTSAMPLE_H
