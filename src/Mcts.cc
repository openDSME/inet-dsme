#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include "Mcts.h"
#include <omnetpp.h>

Mcts::Mcts(float value, int action, Mcts *parent) {
    // set the parameters
    this->parent = parent;
    this->childs.clear();
    this->value = 0.0;
    this->times_node_executed = 0;
    this->action = action;
    if (Init_Mode::Normal == this->initMode) {
        this->uct_score = 99999.0;
    } else {
        this->uct_score = 0.0;
    }
    // init random number generator
    this->randomNumGen = omnetpp::cModule().getRNG(1);
}

Mcts::~Mcts() {
    this->parent = nullptr;
    this->value = 0.0;
    this->times_node_executed = 0;
    this->action = 0;
    this->childs.clear();
}

// delete childs that only are 0
void Mcts::deleteNotUsedNodes() {

    if (this->childs.size() > 0) {

        bool is_all_null = true;
        for (int i = 0; i < this->childs.size(); i++) {

            if (this->childs[i] != nullptr) {

                // if child is null
                if (this->childs[i]->childs.size() == 0) {

                    // has no childs
                    if (this->childs[i]->value <= 0.0) {
                        // value of node is 0 than delete it

                        delete this->childs[i];
                        this->childs[i] = nullptr;
                    }
                } else {

                    // has child nodes, check if all pointers are null (not a leaf node)
                    bool allNull = true;
                    for (int j = 0; j < this->childs[i]->childs.size(); j++) {
                        if (this->childs[i]->childs[j] != nullptr) {
                            allNull = false;
                            is_all_null = false;
                            break;
                        }
                    }
                    if (allNull == true && this->childs[i]->value <= 0.0) {

                        // child can be deleted, is leaf node
                        delete this->childs[i];
                        this->childs[i] = nullptr;
                    }
                }
            }
        }
    }
}

void Mcts::pruneTree() {
    // go to a leaf node
    if (this->childs.size() > 0) {
        // probably not a leaf node
        for (int i = 0; i < this->childs.size(); i++) {
            if (this->childs[i] != nullptr) {
                // call child
                this->childs[i]->pruneTree();
            }
        }
    }
    // check if it can be deleted do that with every node above

    this->deleteNotUsedNodes();

}

float Mcts::uct(int total_num_runs) {
    // Upper Confidence Bound applied to trees
    float exploration_param = 2.0;

    if (this->times_node_executed != 0) {

        this->uct_score =
                (this->value / ((float) this->times_node_executed))
                        + (exploration_param
                                * std::sqrt(
                                        (std::log((float) total_num_runs)
                                                / ((float) this->times_node_executed))));

    } else {

        if (Init_Mode::Normal == this->initMode) {
            this->uct_score = 99999.0;
        } else {

            this->uct_score = 0.0;

        }
    }
    return this->uct_score;
}

void Mcts::createChild(float value, int action) {
    // create a new child node
    Mcts *a = new Mcts(value, action, this);
    this->childs.push_back(a);
}

Mcts* Mcts::currentBestNode(int total_num_runs) {
    // returns a pointer to best leaf node
    if (this->childs.size() > 0) {
        Mcts *bestNode = this->getChild(0);
        float bestScore = 0.0;
        for (int i = 0; i < this->childs.size(); i++) {
            float child_score = this->childs[i]->uct(total_num_runs);
            if (child_score > bestScore) {
                bestScore = child_score;
                bestNode = this->childs[i];
            }
        }
        return bestNode->currentBestNode(total_num_runs);
    } else {
        return this;
    }
}

Mcts* Mcts::getChild(int index) {
    // returns a pointer to the child node that has the action index
    Mcts *child = (this->childs[(index)]);
    return child;
}

Mcts* Mcts::getBestChild() {
    // returns best child node
    int total_num_runs = 1;
    if (this->childs.size() > 0) {
        Mcts *bestNode = this->getChild(0);
        float bestScore = 0.0;
        for (int i = 0; i < this->childs.size(); i++) {
            float child_score = this->childs[i]->uct(total_num_runs);
            if (child_score > bestScore) {
                bestScore = child_score;
                bestNode = this->childs[i];
            }
        }
        return bestNode;
    } else {
        return nullptr;
    }
    return nullptr;
}

int Mcts::getNumChilds() {
    // count of child nodes
    return this->childs.size();
}

int Mcts::checkChildsNonZero() {
    // give back the first child node that is nonzero
    if (this->childs.size() != 0) {
        for (int i = 0; i < this->childs.size(); i++) {
            if (this->childs[i] != nullptr) {
                return i;
            }
        }
    }
    return -1;
}

int Mcts::addNewNode(int value, int action) {
    if (action < this->childs.size()) {
        Mcts *newNode = new Mcts(value, action, this);
        this->childs[action] = newNode;
    }
    return 0;
}

Mcts* Mcts::getMonteCarloChild(int total_num_runs) {
    // return a child node or create child nodes if no exist
    if (this->childs.size() > 0) {
        Mcts *drawnNode = nullptr;
        // there exist child nodes
        int child_Id = checkChildsNonZero();
        if (child_Id > -1) {
            // Mcts *bestNode = this->childs[child_Id];
            float bestScore = 0.0;
            // select next randomly node, with a bias to the current best node
            float sum_values = 0.0;

            // sum up all scores
            for (int i = 0; i < this->childs.size(); i++) {
                // check if child exists
                if (this->childs[i] != nullptr) {
                    // ToDo: iterator kann nicht verwendet werden

                    float child_score = this->childs[i]->uct(total_num_runs);

                    if (child_score > this->percent_take_lower_option) {
                        sum_values += child_score;
                    }
                    if (child_score < this->percent_take_lower_option
                            && child_score >= 0.0) {
                        sum_values += this->percent_take_lower_option;
                    }
                } else {
                    sum_values += this->percent_take_lower_option;
                }
            }

            // calculate the probability for each
            float number = omnetpp::uniform(this->randomNumGen, 0.0, 1.0);
            float counter_score = 0.0;
            drawnNode = this->childs[0];
            // make a decision
            for (int i = 0; i < this->childs.size(); i++) {
                if (this->childs[i] != nullptr) {
                    // if child exists test if value is below threshold
                    if (this->childs[i]->uct_score
                            <= this->percent_take_lower_option
                            && this->childs[i]->uct_score >= 0.0) {
                        counter_score += this->percent_take_lower_option
                                / sum_values;
                    } else {
                        counter_score += this->childs[i]->uct_score
                                / sum_values;
                    }
                    if (counter_score >= number) {
                        // drawn node with weight
                        drawnNode = this->childs[i];
                        break;
                    }
                } else {
                    // if node does not exist add the minimum amount
                    counter_score += this->percent_take_lower_option
                            / sum_values;
                    if (counter_score >= number) {
                        // drawn node with weight and create that node new

                        this->addNewNode(0.0, i);
                        drawnNode = this->childs[i];
                        break;
                    }
                }
            }

        } else {
            // all childs are null
            // create a child node for a random action

            int number = omnetpp::intuniform(this->randomNumGen, 0,
                    this->childs.size() - 1);

            this->addNewNode(0.0, number);
            drawnNode = this->childs[number];

        }

        return drawnNode;
    } else {
        // when no child nodes exist, create child nodes and choose one randomly
        int numchilds = this->parent->childs.size();
        for (int i = 0; i < numchilds; i++) {
            this->createChild(0.0, i);
        }

        // delete not used
        this->deleteNotUsedNodes();
        // select one randomly using the omnetpp random number generator
        int number = omnetpp::intuniform(this->randomNumGen, 0, numchilds - 1);
        this->addNewNode(0.0, number);
        Mcts *childnode = this->getChild(number);

        return childnode;
    }

    return nullptr;
}

Mcts* Mcts::getPointer() {
    // returns the pointer of the current node
    return this;
}

void Mcts::printTree(int level) {
    // print whole tree that is below this node
    if (level == 0) {
        std::cout << "current best " << this->getBestAction() << std::endl;
        std::cout << "action | ubc1_score | times_executed | value"
                << std::endl;
    }
    for (int i = 0; i < level; i++) {
        std::cout << " ";
    }
    std::cout << this->action << " " << this->uct_score << " "
            << this->times_node_executed << " " << this->value << std::endl;
    level++;
    for (auto child : this->childs) {
        // run the printTree method with all child nodes
        if (child != nullptr) {
            child->printTree(level);
        }
    }
}

void Mcts::backpropagateScore(float value) {
    // backpropagate score through the tree
    if (this->times_node_executed == 0) {
        this->value = value;
        this->times_node_executed = 1;
    } else {
        this->value += value;
        if (this->value < 0.0) {
            this->value = 0.0;
        }
        this->times_node_executed++;
    }
    if (this->parent != nullptr) {
        this->parent->backpropagateScore(value);
    }
}

void Mcts::backpropagateScore_RL(float value) {
    // backpropagate score but use a weighted summation
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
    // get the best action in the current state
    float bestValue = -99999.0;
    int bestAction = 0;
    for (auto child : this->childs) {
        if (child != nullptr) {
            if (child->value > bestValue) {
                bestValue = child->value;
                bestAction = child->action;
            }
        }
    }
    return bestAction;
}

std::vector<int> Mcts::getActionsPerformed() {
    // returns a vector of the actions that were performed to end up in this node
    std::vector<int> actionsPerformed;
    actionsPerformed.clear();
    if (this->parent != nullptr) {
        actionsPerformed = this->parent->getActionsPerformed();
        actionsPerformed.push_back(this->action);
    }
    return actionsPerformed;
}

int Mcts::getAction() {
    // return the action that is represented by this node
    return this->action;
}

int Mcts::getLayer() {
    // calculate the layer of the tree that this node is on
    if (this->parent != nullptr) {
        return this->parent->getLayer() + 1;
    }
    return 0;
}

Mcts* Mcts::getRootNode() {
    // get the root node of the tree
    if (this->parent != nullptr) {
        return this->parent->getRootNode();
    }
    return this;
}

Mcts* Mcts::getNodeforState(int layer, int total_num_runs) {

    // find the node to a given layer
    // retrieve the current layer
    int current_layer = this->getLayer();

    // test if it is this layer
    if (current_layer == layer) {

        return this;
    }

    // maybe it is deeper
    if (current_layer < layer) {
        // use the monte carlo child method to find good nodes on the way down
        Mcts *layer_node = this;

        while (layer_node->getLayer() < layer) {

            layer_node = layer_node->getMonteCarloChild(total_num_runs);

        }

        return layer_node;
    }
    // not so deep
    // start at the top again
    Mcts *root = this->getRootNode();

    for (int i = 0; i < layer; i++) {
        // work the way down the tree
        root = root->getMonteCarloChild(total_num_runs);
    }

    return root;
}

