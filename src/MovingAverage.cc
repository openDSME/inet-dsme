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
// 


#include <iostream>
#include <vector>
#include <cmath>
#include <omnetpp.h>
#include "MovingAverage.h"

MovingAverage::MovingAverage(int windowSize, std::string name){
    this->loggingAverage = new cOutVector((name + ":average").data());
    this->loggingweightedAverage = new cOutVector((name + ":weightedAverage").data());
    this->windowSize = windowSize;
    this->values.clear();
}

MovingAverage::~MovingAverage(){
    this->values.clear();
}

float MovingAverage::getAverage(){
    return this->average;
}

void MovingAverage::print(){
    for (int i = 0; i < this->values.size(); i++)
    {
        std::cout << std::to_string((int)this->values.at(i));
    }
    std::cout << std::endl;
}

float MovingAverage::weightedAverage(){
    float sum = 0;
    for (int i = 0; i < this->values.size(); i++)
    {
         sum += this->values.at(i) / std::pow(2.0, this->values.size() - i);
    }
    sum += this->values.front() / std::pow(2.0, this->values.size());
    this->loggingweightedAverage->record(sum);
    return sum;
}

void MovingAverage::newValue(float value){
    // add new value
    this->values.push_back(value);
    if (this->values.size() > this->windowSize){
        this->values.erase(this->values.begin());
    }

    // calc average of new window
    float sum = 0.0;
    for (int i = 0; i < this->values.size(); i++)
    {
        sum += this->values.at(i);
    }
    this->average = (float)(sum) / (float)(this->values.size());
    this->loggingAverage->record(this->average);
}

void MovingAverage::reset(int windowSize){
    this->average = 0.0;
    this->values.clear();
    this->windowSize = windowSize;
}


