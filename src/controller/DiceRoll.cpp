//
// Created by danilo on 5/14/20.
//

#include "../../include/DiceRoll.h"

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
    return numAces;
}

void DiceRoll::setNumAces(int nNumAces) {
    this->numAces = nNumAces;
}

const std::vector<int> &DiceRoll::getDice() const {
    return dice;
}

void DiceRoll::setDice(const std::vector<int> &nDice) {
    this->dice = nDice;
}
