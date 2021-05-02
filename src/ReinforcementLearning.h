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

#ifndef SRC_REINFORCEMENTLEARNING_H_
#define SRC_REINFORCEMENTLEARNING_H_

#include <cmath>
#include <iostream>
#include <omnetpp.h>
#include "QLearning.h"
#include "Mcts.h"

//namespace inet_dsme {
// class Mcts;

using omnetpp::cOutVector;

class ReinforcementLearning {
private:
    enum Algos {Qlearning, MCts};
    int algo = Algos::Qlearning;
    int states = 8;
    int actions = 9;
    float transmissionPowers[10] = {0.0, 0.01, 0.1, 1.0, 2.24, 5.0, 10.0, 15.0, 20.0, 25.0};
    int state_current = 0;
    int state_next = 0;
    int state_last = 0;
    int action_current = 0;
    int action_last = 0;
    int action_next = 5;
    int slots = 4;
    QLearning *QLearningClass;
    Mcts *MctsClass;

    cOutVector *logging_transmissionpower;
    cOutVector *logging_states;
    cOutVector *logging_dataStatus;

    float reward(int action, float prr_last);
    int prr_to_state(float prr_last);
public:
    ReinforcementLearning();
    // ReinforcementLearning(int states = 8);
    virtual ~ReinforcementLearning();

    double getPower();
    int getBestAction();
    int rewardAction(float prr_last, int datastatus);
    void logPower(double power);
    void print();
    int getCurrentState();
    int getCurrentAction();
//    int currentState
//    int predictState
//    int pickbestAction
    enum StateTransitions {Prr, Packets, SuperFrame};
    void setSuperFrameState(int superframe, int slot);
    int getStateTranstion();

    enum ReinforementLearningOptions {Normal, Learning};
    int getReinforcementLearningOption();
    void setReinforcementLearningOption(int option);

    // QLearning Implementation
    void initQLearning();
    void initQLearning(int states);

    // Mcts
    void initMcts();

private:
    // Options
    int ReinforementLearningOption = ReinforementLearningOptions::Learning;
    int state_transition = StateTransitions::SuperFrame;
};

//} /* namespace inet_dsme */

#endif /* SRC_REINFORCEMENTLEARNING_H_ */
