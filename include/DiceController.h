//
// Created by danilo on 5/14/20.
//
#ifndef DICE_CONTROLLER_H
#define DICE_CONTROLLER_H
#include <vector>
#include <string>
#include "DiceRoll.h"
#include <random>

class DiceController {
public:
    DiceController();
    void logRolls(const std::string& rolls);
    void clearLog();
    bool isAcing() const;
    void toggleAces();
    std::vector<DiceRoll*> *getAllRolls(char *roll);

private:
    //vars
    std::random_device randomDevice;
    std::mt19937 twisterEngine;
    bool aces;
    const char* LOG_NAME = "RollHistory.txt";

    //functions
    int getRoll(int);
    std::vector<int> *parseRoll(char*);

};


#endif
