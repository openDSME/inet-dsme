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
    return ((this->actions - action) * 10) * prr_last;
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

void ReinforcementLearning::setSuperFrameState(int superframe, int slot){
   //  int num = superframe * this->slots + slot;
    this->state_next = superframe * this->slots + slot;
}

int ReinforcementLearning::getStateTranstion(){
    return state_transition;
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
        // best Action
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
        // get reward
        float reward = this->reward(this->action_current, prr_last);
        // recalc QTable
        this->QLearningClass->updateQTable(this->action_current,
                this->state_current, this->state_next, reward);
        // guess next step
        this->action_next = this->QLearningClass->getAction(this->state_next);
    }
    // if Mcts
    if (this->algo == Algos::MCts) {
        // current best Node
        // create all possible childs
        // backpropagate Score
        // update score *0.3 new + *0.7 old score
        // best Action
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
}

int ReinforcementLearning::getCurrentAction() {
    return this->action_next;
}

int ReinforcementLearning::getCurrentState() {
    return this->state_next;
}

// use QLearning
void ReinforcementLearning::initQLearning() {
    this->algo = Algos::Qlearning;
    this->QLearningClass = new QLearning(this->states, this->actions);
}
void ReinforcementLearning::initQLearning(int states) {
    this->states = states;
    this->initQLearning();
}

// use Mcts
void ReinforcementLearning::initMcts() {
    this->algo = Algos::MCts;
    this->MctsClass = new Mcts(0.0, this->actions);
}

int ReinforcementLearning::getReinforcementLearningOption() {
    return this->ReinforementLearningOption;
}

void ReinforcementLearning::setReinforcementLearningOption(int option) {
    this->ReinforementLearningOption = option;
}

//} /* namespace inet_dsme */
