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
    enum Algos {Qlearning, MCts, NormalAlgo};
    int algo = Algos::NormalAlgo;
    int states = 8;
    int actions = 11;
    float transmissionPowers[11] = {0.0, 0.01, 0.05, 0.1, 0.2, 0.3, 0.5, 0.7, 1.0, 1.5, 2.24};
    int state_current = 0;
    int state_next = 0;
    int state_last = 0;
    int action_current = 0;
    int action_last = 0;
    int action_next = 5;
    int total_num_runs = 0;
    int slots = 16;
    float reward_last = 0.0;
    float epsilon_set_back = 0.8;
    QLearning *QLearningClass;
    Mcts *MctsClass; // max states deep
    Mcts *MctsState;

    cOutVector *logging_transmissionpower;
    cOutVector *logging_states;
    cOutVector *logging_dataStatus;

    float reward(int action, float prr_last);
    float neg_reward(int action, float prr_last);
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
    void setStateTransition(std::string transition);

    enum ReinforementLearningOptions {Normal, Learning};
    int getReinforcementLearningOption();
    void setReinforcementLearningOption(int option);

    // QLearning Implementation
    void initQLearning();
    void initQLearning(int states);
    void setupQLearning(double alpha, double gamma, double epsilon, double epsilon_percentage, double epsilon_set_back, double min_epsilon, bool is_greedy, bool is_hotbooting);

    // Mcts
    void initMcts();
    void initMcts(int states);

private:
    // Options
    int ReinforementLearningOption = ReinforementLearningOptions::Normal;
    int state_transition = StateTransitions::Packets;
};

//} /* namespace inet_dsme */

#endif /* SRC_REINFORCEMENTLEARNING_H_ */
