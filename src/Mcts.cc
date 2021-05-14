#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include "Mcts.h"
#include <omnetpp.h>

Mcts::Mcts(float value, int action, Mcts *parent) {
    this->parent = parent;
    this->childs.clear();
    this->value = 0.0;
    this->times_node_executed = 0;
    this->action = action;
    if (Init_Mode::Normal == this->initMode) {
        this->ucb1_score = 99999.0;
    } else {
        this->ucb1_score = 0.0;
    }
    // init random number generator
    this->randomNumGen = omnetpp::cModule().getRNG(1);
}

Mcts::~Mcts() {
}

float Mcts::ucb1(int total_num_runs) {
    // Upper Confidence Bound
    float exploration_param = 2.0;
    if (this->times_node_executed != 0) {
        this->ucb1_score =
                (this->value / ((float) this->times_node_executed))
                        + (exploration_param
                                * std::sqrt(
                                        (std::log((float) total_num_runs)
                                                / ((float) this->times_node_executed))));
    } else {
        if (Init_Mode::Normal == this->initMode) {
            this->ucb1_score = 99999.0;
        } else {
            this->ucb1_score = 0.0;
        }
    }
    return this->ucb1_score;
}

void Mcts::createChild(float value, int action) {
    Mcts *a = new Mcts(value, action, this);
    this->childs.push_back(*a);
}

Mcts* Mcts::currentBestNode(int total_num_runs) {
    // return best Child node
    if (this->childs.size() > 0) {
        Mcts *bestNode = this->getChild(0);
        float bestScore = 0.0;
        for (std::vector<Mcts>::iterator child = this->childs.begin();
                child != this->childs.end(); ++child) {
            float child_score = child->ucb1(total_num_runs);
            if (child_score > bestScore) {
                bestScore = child_score;
                bestNode = child->getPointer();
            }
        }
        return bestNode->currentBestNode(total_num_runs);
    } else {
        return this;
    }
}

Mcts* Mcts::getChild(int index) {
    // std::cout << "get child " << std::endl;
    Mcts *child = &(this->childs[(index)]);
    // std::cout << " got child " << std::endl;
    return child;
}

Mcts* Mcts::getBestChild() {
    // return best Child node
    int total_num_runs = 1;
    if (this->childs.size() > 0) {
        Mcts *bestNode = this->getChild(0);
        float bestScore = 0.0;
        for (std::vector<Mcts>::iterator child = this->childs.begin();
                child != this->childs.end(); ++child) {
            float child_score = child->ucb1(total_num_runs);
            if (child_score > bestScore) {
                bestScore = child_score;
                bestNode = child->getPointer();
            }
        }
        return bestNode;
    } else {
        return nullptr;
    }
    return nullptr;
}

int Mcts::getNumChilds() {
    return this->childs.size();
}

Mcts* Mcts::getMonteCarloChild() {
    // return best Child node or create childs
    int total_num_runs = 1; // TODO : just for testing
    if (this->childs.size() > 0) {
        // std::cout << " montecarlo get child " << std::endl;
        Mcts *bestNode = this->getChild(0);
        float bestScore = 0.0;
        // select current best or select random
        // sum up all values
        float sum_values = 0.0;
        for (std::vector<Mcts>::iterator child = this->childs.begin();
                child != this->childs.end(); ++child) {
            float child_score = child->ucb1(total_num_runs);
            if (child_score > 0.0) {
                sum_values += child_score;
            }
            if(child_score < this->percent_take_lower_option && child_score >= 0.0){
                sum_values += this->percent_take_lower_option;
            }
        }

        // calc probability for each
        float number = omnetpp::uniform(this->randomNumGen, 0.0, 1.0);
        float counter_score = 0.0;
        Mcts *drawnNode = this->childs[0].getPointer();
        // make a decision
        for (std::vector<Mcts>::iterator child = this->childs.begin();
                child != this->childs.end(); ++child) {
            counter_score += child->value / sum_values;
            if(child->value < this->percent_take_lower_option && child->value >= 0.0){
                counter_score += this->percent_take_lower_option;
            }
            if (counter_score >= number) {
                // drawn node with weight
                drawnNode = child->getPointer();
                child = this->childs.end();
                break;
            }
        }

//        for (std::vector<Mcts>::iterator child = this->childs.begin();
//                child != this->childs.end(); ++child) {
//            // select current best child or select random child
//            float child_score = child->ucb1(total_num_runs);
//
//            // TODO: hier noch einfügen
//
//            // if better change choise
//            if (child_score > bestScore) {
//                bestScore = child_score;
//                bestNode = child->getPointer();
//            }
//
//        }
//        return bestNode;
        return drawnNode;
    } else {
        // std::cout << " montecarlo new layer " << std::endl;
        // create childs and choose one randomly
        int numchilds = this->parent->childs.size();
        for (int i = 0; i < numchilds; i++) {
            this->createChild(0.0, i);
        }
        // std::cout << " all childs " << std::endl;
        // select one randomly
        // std::cout << " numchilds " << numchilds << std::endl;
        int number = omnetpp::intuniform(this->randomNumGen, 0, numchilds - 1);
        // std::cout << " child selected " << number << std::endl;
        Mcts *childnode = this->getChild(number);
        // std::cout << "child node action " << childnode->getAction()
         //       << std::endl;
        return childnode;
    }
    // will never happen
    return nullptr;
}

Mcts* Mcts::getPointer() {
    return this;
}

void Mcts::printTree(int level) {
    if (level == 0) {
        std::cout << "current best " << this->getBestAction() << std::endl;
        std::cout << "action | ubc1_score | times_executed | value"
                << std::endl;
    }
    for (int i = 0; i < level; i++) {
        std::cout << " ";
    }
    std::cout << this->action << " " << this->ucb1_score << " "
            << this->times_node_executed << " " << this->value << std::endl;
    level++;
    for (auto &child : this->childs) {
        // std::cout << "ac" << child.getAction() << std::endl;
        child.printTree(level);
    }
}

void Mcts::backpropagateScore(float value) {
    if (this->times_node_executed == 0) {
        this->value = value;
        this->times_node_executed = 1;
    } else {
        this->value += value;
        if(this->value < 0.0){
            this->value = 0.0;
        }
        this->times_node_executed++;
    }
    if (this->parent != nullptr) {
        this->parent->backpropagateScore(value);
    }
}

void Mcts::backpropagateScore_RL(float value) {
    if (this->times_node_executed == 0) {
        this->value = (1.0 - this->oldValuestrength) * value
                + this->oldValuestrength * this->value;
        this->times_node_executed = 1;
    } else {
        this->value = (1.0 - this->oldValuestrength) * value
                + this->oldValuestrength * this->value;
        this->times_node_executed++;
    }
    if (this->parent != nullptr) {
        this->parent->backpropagateScore_RL(value);
    }
}

int Mcts::getBestAction() {
    float bestValue = -99999.0;
    int bestAction = 0;
    for (auto &child : this->childs) {
        if (child.value > bestValue) {
            bestValue = child.value;
            bestAction = child.action;
        }
    }
    return bestAction;
}

std::vector<int> Mcts::getActionsPerformed() {
    std::vector<int> actionsPerformed;
    actionsPerformed.clear();
    if (this->parent != nullptr) {
        actionsPerformed = this->parent->getActionsPerformed();
    }
    if (this->parent != nullptr) {
        actionsPerformed.push_back(this->action);
    }
    return actionsPerformed;
}

int Mcts::getAction() {
    // std::cout << "testing Action" << std::endl;
    // std::cout << "get Action" << this->action << std::endl;
    return this->action;
}

int Mcts::getLayer() {
    // std::cout << "getlayer" << std::endl;
    if (this->parent != nullptr) {
        // std::cout << "1" << std::endl;
        return this->parent->getLayer() + 1;
    }
    // std::cout << "getlayer top" << std::endl;
    return 0;
}

Mcts* Mcts::getRootNode() {
    // get the root of the tree
    if (this->parent != nullptr) {
        return this->parent->getRootNode();
    }
    return this;
}

Mcts* Mcts::getNodeforState(int layer) {
    // std::cout << "layer " << layer << std::endl;
    // myLayer
    int current_layer = this->getLayer();
    // std::cout << " currentlayer " << current_layer << std::endl;
    if (current_layer == layer) {
        return this;
    }

    // is deeper
    if (current_layer < layer) {
        // std::cout << "deeper " << std::endl;
        // while currentlayer < layer
        Mcts *layer_node = this;
        while (layer_node->getLayer() < layer) {
            // std::cout << "> deeper " << std::endl;
            layer_node = layer_node->getMonteCarloChild();
        }
        return layer_node;
    }
    // not so deep
    // start at the top again
    Mcts *root = this->getRootNode();
    for (int i = 0; i < layer; i++) {
        // work the way down the tree
        root = root->getMonteCarloChild();
    }
    return root;
}

