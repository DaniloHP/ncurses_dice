//
// Created by danilo on 5/14/20.
//

#include "../../include/DiceRoll.h"

DiceRoll::DiceRoll() {
    sum = dieType = reps = origReps = numAces = 0;
    rolls = new std::vector<int>();
}

DiceRoll::~DiceRoll() {
    rolls->clear();
    delete rolls;
}

int DiceRoll::getSum() const {
    return sum;
}

void DiceRoll::setSum(int nSum) {
    this->sum = nSum;
}

int DiceRoll::getDieType() const {
    return dieType;
}

void DiceRoll::setDieType(int nDieType) {
    this->dieType = nDieType;
}

int DiceRoll::getReps() const {
    return reps;
}

void DiceRoll::setReps(int nReps) {
    this->reps = nReps;
}

int DiceRoll::getOrigReps() const {
    return origReps;
}

void DiceRoll::setOrigReps(int nOrigReps) {
    this->origReps = nOrigReps;
}

int DiceRoll::getNumAces() const {
    return reps - origReps;
}

int DiceRoll::getAt(int i) const {
    if (i < rolls->size())
        return rolls->at(i);
    else
        return -1;
}

void DiceRoll::pushBack(int val) {
    rolls->push_back(val);
}
