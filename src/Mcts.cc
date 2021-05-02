
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include "Mcts.h"

Mcts::Mcts(float value, int action, Mcts *parent)
{
    this->parent = parent;
    this->childs.clear();
    this->value = 0.0;
    this->times_node_executed = 0;
    this->action = action;
    this->ucb1_score = 9999999;
    //return this;
}

Mcts::~Mcts()
{
}

float Mcts::ucb1(int total_num_runs){
    // Upper Confidence Bound
    float exploration_param = 2;
    if (this->times_node_executed != 0){
        this->ucb1_score = (this->value / this->times_node_executed) +
        (exploration_param * std::sqrt((std::log(total_num_runs) / this->times_node_executed)));
    }else{
        this->ucb1_score = 9999999;
    }
    return this->ucb1_score;
}

void Mcts::createChild(float value, int action){
    Mcts* a = new Mcts(value, action, this);
    this->childs.push_back(*a);
}

Mcts* Mcts::currentBestNode(int total_num_runs){
    // return best Child node
    if (this->childs.size() > 0){
        Mcts* bestNode = this->getChild(0);
        float bestScore = 0.0;
        for(std::vector<Mcts>::iterator child = this->childs.begin(); child != this->childs.end(); ++child){
            float child_score = child->ucb1(total_num_runs);
            if (child_score > bestScore){
                bestScore = child_score;
                bestNode = child->getPointer();
            }
        }
        return bestNode->currentBestNode(total_num_runs);
    }else{
        return this;
    }
}

Mcts* Mcts::getChild(int index){
    return &this->childs.at(index);
}

Mcts* Mcts::getPointer(){
    return this;
}

void Mcts::printTree(int level){
    for(int i = 0; i < level; i++){
        std::cout << " ";
    }
    std::cout << this->action << " " << this->ucb1_score << " " << this->times_node_executed << " " << this->value << std::endl;
    level++;
    for(auto &child : this->childs){
    // std::cout << "ac" << child.getAction() << std::endl;
        child.printTree(level);
    }
}

void Mcts::backpropagateScore(float value){
    if(this->times_node_executed == 0){
        this->value = value;
        this->times_node_executed = 1;
    }else{
        this->value += value;
        this->times_node_executed++;
    }
    if(this->parent != nullptr){
        this->parent->backpropagateScore(value);
    }
}

int Mcts::getBestAction(){
    float bestValue = -9999999;
    int bestAction = 0;
    for (auto &child : this->childs){
        if(child.value > bestValue){
            bestValue = child.value;
            bestAction = child.action;
        }
    }
    return bestAction;
}

std::vector<int> Mcts::getActionsPerformed(){
    std::vector<int> actionsPerformed;
    actionsPerformed.clear();
    if(this->parent != nullptr){
        actionsPerformed = this->parent->getActionsPerformed();
    }
    if(this->parent != nullptr){
        actionsPerformed.push_back(this->action);
    }
    return actionsPerformed;
}

int Mcts::getAction(){
    return this->action;
}



