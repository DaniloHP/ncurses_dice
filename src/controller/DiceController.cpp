//
// Created by danilo on 5/14/20.
//

#include "../../include/DiceController.h"
#include <random>
#include <fstream>
#include <string>

DiceController::DiceController() {
    aces = true; //eventually read from a file or something
}

int DiceController::getRoll(int dieType) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1, dieType);
    return dist(mt);
}

std::vector<int>* DiceController::parseRoll(char *roll) {
    auto *arr = new std::vector<int>();
    std::string s = roll;
    int num = 0;
    for (char c : s) {
        if ((c == 'd' || c == 'D') && num == 0) {
            arr->push_back(1);
        } else if (c > 47 && c < 58) {
            num = num * 10 + (c - 48);
        } else if (num > 0) {
            arr->push_back(num);
            num = 0;
        }
    }
    if (num > 0) {
        arr->push_back(num);
    }
    s.erase(0);
    return arr;
}

void DiceController::logRolls(const std::string& rolls) {
    time_t now = time(nullptr);
    std::string date = ctime(&now);
    std::ofstream file;
    file.open(logName, std::ios_base::app);
    file << date + rolls << std::endl;
    file.close();
    date.erase(0);
}

void DiceController::clearLog() {
    std::ofstream file;
    file.open(logName, std::ofstream::out | std::ofstream::trunc);
    file.close();
}

//TODO vector of DiceRoll pointers? And then freeing them when necessary?
std::vector<DiceRoll*> *DiceController::getAllRolls(char *roll) {
    auto *allRolls = new std::vector<DiceRoll*>();
    std::vector<int> *rollNums = parseRoll(roll);
    int sum, origReps, dieType, reps, rollVal, i, j;
    for (i = 0; i < rollNums->size(); i += 2) {
        auto currRoll = new DiceRoll();
        sum = 0;
        dieType = rollNums->at(i + 1);
        reps = origReps = rollNums->at(i);
        currRoll->setDieType(dieType);
        currRoll->setOrigReps(origReps);
        for (j = 0; j < reps; j++) {
            rollVal = getRoll(dieType);
            currRoll->pushBack(rollVal);
            sum += rollVal;
            if (aces && rollVal == dieType && dieType != 1) reps++;
        }
        currRoll->setReps(reps);
        currRoll->setSum(sum);
        allRolls->push_back(currRoll);
    }
    delete rollNums;
    return allRolls;
}

bool DiceController::isAcing() const {
    return aces;
}

void DiceController::toggleAces() {
    aces = !aces;
}
