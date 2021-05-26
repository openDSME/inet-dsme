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

#include "ReinforcementLearning.h"
#include <cmath>
#include <iostream>
#include <omnetpp.h>
#include "QLearning.h"
#include "Mcts.h"

//namespace inet_dsme {

ReinforcementLearning::ReinforcementLearning() {
    // TODO Auto-generated constructor stub
    this->logging_transmissionpower = new cOutVector("TransmissionPower");
    this->logging_states = new cOutVector("States");
    this->logging_dataStatus = new cOutVector("DataStatus");

    // set TransmissionPower to x
    //    this->transmissionPowers = {0.01,0.1,1.0,2.24,5.0};
}

ReinforcementLearning::~ReinforcementLearning() {
}

// interface with learning algo
//void ReinforcementLearning::Linear_Value_to_States(float value){
//    int maxvalue
//}

float ReinforcementLearning::reward(int action, float prr_last) {
    // as little as possible Transmissionpower as much reward as possible
    return ((this->actions - action) * 1.0) * prr_last;
    // return prr_last;
}

float ReinforcementLearning::neg_reward(int action, float prr_last) {
    // as little as possible Transmissionpower as much reward as possible
    // can be negative
    return (((this->actions - action) * 1.0) * (prr_last-0.3));
    // return prr_last;
}

void ReinforcementLearning::logPower(double power = 0.0) {
    if (this->ReinforementLearningOption
            == ReinforementLearningOptions::Normal) {
        this->logging_transmissionpower->record(power);
    } else {
        this->logging_transmissionpower->record(this->getPower());
    }
}

int ReinforcementLearning::prr_to_state(float prr_last) {
    int state = std::floor(prr_last * this->states);
    if (state >= this->states) {
        state = this->states - 1;
    }
    return state;
}

void ReinforcementLearning::setSuperFrameState(int superframe, int slot) {
    //  int num = superframe * this->slots + slot;
    this->state_next = superframe * this->slots + slot;
    // std::cout << "slot" << slot << " frame " << superframe <<std::endl;
}

int ReinforcementLearning::getStateTranstion() {
    return state_transition;
}

void ReinforcementLearning::setStateTransition(std::string transition){
    // set the statetransition
    if(transition == "SuperFrame"){
        this->state_transition = this->StateTransitions::SuperFrame;
    }
    if(transition == "Packets"){
            this->state_transition = this->StateTransitions::Packets;
        }
    if(transition == "Prr"){
            this->state_transition = this->StateTransitions::Prr;
       }
}

double ReinforcementLearning::getPower() {
    return this->transmissionPowers[this->action_next];
}

int ReinforcementLearning::getBestAction() {
    // how to select state
    if (this->state_transition == StateTransitions::Packets) {
        this->state_next = (this->state_next + 1) % this->states;
    }

    // if Qlearning
    if (this->algo == Algos::Qlearning) {
        // guess next step
        this->action_next = this->QLearningClass->getAction(this->state_next);
    }
    // if Mcts
    if (this->algo == Algos::MCts) {
        // best Action in current Layer
        // this->action_next = this->MctsState->getBestAction();
        // std::cout << "get next action" << std::endl;
        Mcts *mctsnode = this->MctsState->getNodeforState(this->state_next);
        // std::cout << "get monte carlo child" << std::endl;
        Mcts *mctschildnode = mctsnode->getMonteCarloChild();
        // std::cout << "bestAction is action " << mctschildnode << " child node layer " << mctschildnode->getLayer() << std::endl;
        this->action_next  = mctschildnode->getAction();
        // std::cout << "bestAction mcts finished "<<std::endl;
    }
    // old state = new state
    this->action_last = this->action_current;
    this->action_current = this->action_next;

    this->state_last = this->state_current;
    this->state_current = this->state_next;
    // this->logging_transmissionpower->record(this->getPower());
    this->logging_states->record(this->state_next);
    return this->action_next;
}

int ReinforcementLearning::rewardAction(float prr_last, int datastatus) {
    // how to select state
    if (this->state_transition == StateTransitions::Prr) {
        this->state_next = this->prr_to_state(prr_last);
    }
    if (this->state_transition == StateTransitions::Packets) {
        this->state_next = (this->state_next + 1) % this->states;
    }

    // if Qlearning
    if (this->algo == Algos::Qlearning) {
        // if prr is zero then try to reset the learning rate
        if (prr_last == 0.0) {
            this->QLearningClass->setLearningRate(this->epsilon_set_back, true);
            // this->QLearningClass->setEpsilonDynamicIncrease(); -> is worse
        }
        // get reward
        float reward = this->reward(this->action_current, prr_last);

        // recalc QTable
        this->QLearningClass->updateQTable(this->action_current,
                this->state_current, this->state_next, this->reward_last);
        // guess next step
        this->action_next = this->QLearningClass->getAction(this->state_next);
        this->reward_last = reward;
    }
    // if Mcts
    if (this->algo == Algos::MCts) {
        int maxTreeDepth = this->states;
        this->total_num_runs++;
        // std::cout << "reward" << std::endl;
        // get current best node n layers deep
        Mcts *currentNode = this->MctsClass->getNodeforState(this->state_current);
        // update values
        // negative reward
        // std::cout << "reward backprop" << std::endl;
        currentNode->backpropagateScore(this->neg_reward(this->action_current, prr_last));

//        // current best Node at a Leaf
//        Mcts *bestChild = this->MctsClass->currentBestNode(
//                this->total_num_runs);
//        // create all possible childs
//        std::vector<int> actionsPerformed = bestChild->getActionsPerformed();
//        if (actionsPerformed.size() < maxTreeDepth) {
//            // es können noch nodes eingehängt werden
//            for (int i = 0; i < this->actions; i++) {
//                bestChild->createChild(0.0, i);
//            }
//        }
//        // backpropagate Score
//        bestChild->backpropagateScore_RL(prr_last);
//        // update score *0.3 new + *0.7 old score
//        // best Action
//        this->MctsClass->printTree();
//        //this->MctsState = bestChild;
//        if(this->state_next != this->MctsState->getLayer()){
//            // if current layer is not the State
//            if(this->MctsState->getLayer() < this->state_next){
//                // go layers down till reached
//                for(int i = 0; i < this->states; i++){
//                    this
//                }
//            }else{
//                // start at the top again
//                this->MctsState = this->MctsClass;
//            }
//        }
        // this->action_next = this->MctsClass->getBestAction();
        // this->MctsClass->printTree();
    }
    // old state = new state
    this->action_last = this->action_current;
    this->action_current = this->action_next;

    this->state_last = this->state_current;
    this->state_current = this->state_next;
    // this->logging_transmissionpower->record(this->getPower());
    this->logging_states->record(this->state_next);
    this->logging_dataStatus->record(datastatus);
    return this->action_next;
}

void ReinforcementLearning::print() {
    std::cout << " CS: " << this->state_next << " CA: " << this->action_next
            << std::endl;
    if (this->algo == Algos::Qlearning) {
        this->QLearningClass->print_q_table();
    }
    if (this->algo == Algos::MCts) {
        this->MctsClass->printTree();
    }
}

int ReinforcementLearning::getCurrentAction() {
    return this->action_next;
}

int ReinforcementLearning::getCurrentState() {
    return this->state_next;
}

// use QLearning
void ReinforcementLearning::initQLearning() {
    this->ReinforementLearningOption = ReinforementLearningOptions::Learning;
    this->algo = Algos::Qlearning;
    this->QLearningClass = new QLearning(this->states, this->actions);
}
void ReinforcementLearning::initQLearning(int states) {
    this->states = states;
    this->initQLearning();
}
void ReinforcementLearning::setupQLearning(double alpha, double gamma, double epsilon, double epsilon_percentage, double epsilon_set_back, double min_epsilon, bool is_greedy, bool is_hotbooting){
    this->QLearningClass->updateAllParameters(alpha, gamma, epsilon, epsilon_percentage, min_epsilon, is_greedy, is_hotbooting);
    this->epsilon_set_back = epsilon_set_back;
}

// use Mcts
void ReinforcementLearning::initMcts() {
    this->ReinforementLearningOption = ReinforementLearningOptions::Learning;
    this->algo = Algos::MCts;
    this->MctsClass = new Mcts(0.0, this->actions);
    this->MctsState = this->MctsClass;
    // input all possible childs
    for (int i = 0; i < this->actions; i++) {
        this->MctsClass->createChild(0.0, i);
    }
}

void ReinforcementLearning::initMcts(int states) {
    this->states = states;
    this->initMcts();
}

int ReinforcementLearning::getReinforcementLearningOption() {
    return this->ReinforementLearningOption;
}

void ReinforcementLearning::setReinforcementLearningOption(int option) {
    this->ReinforementLearningOption = option;
}

//} /* namespace inet_dsme */
