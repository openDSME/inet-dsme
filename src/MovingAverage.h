//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.


#ifndef SRC_MOVINGAVERAGE_H_
#define SRC_MOVINGAVERAGE_H_

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <omnetpp.h>
// #include <coutvector.h>

using omnetpp::cOutVector;

class MovingAverage
{
private:
    int windowSize = 5;
    float average = 0.0;
    std::vector<float> values;
    cOutVector *loggingAverage;
    cOutVector *loggingweightedAverage;

public:
    MovingAverage(int windowSize = 5, std::string name = "test");
    ~MovingAverage();
    void print();
    void newValue(float value);
    float getAverage();
    float weightedAverage();
    void reset(int windowSize = 5);
};


#endif /* SRC_MOVINGAVERAGE_H_ */
