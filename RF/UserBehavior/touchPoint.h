//
// Created by qhh on 2020/1/6.
//

#ifndef RF_TOUCHPOINT_H
#define RF_TOUCHPOINT_H


struct touchPoint {
    long times_stamp;
    double x;
    double y;
    double pressure;
    double area_covered;
    double phone_orientation;
};


#endif //RF_TOUCHPOINT_H
