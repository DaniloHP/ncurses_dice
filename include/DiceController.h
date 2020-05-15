//
// Created by danilo on 5/14/20.
//
#ifndef DICE_CONTROLLER_H
#define DICE_CONTROLLER_H
#include <vector>
#include <string>
#include "DiceRoll.h"

class DiceController {
public:
    DiceController();
    void logRolls(const std::string& rolls);
    void clearLog();
    bool isAcing() const;
    void toggleAces();
    std::vector<DiceRoll*> *getAllRolls(char *roll);

private:
    int getRoll(int);
    std::vector<int> *parseRoll(char*);
    bool aces;

private:
    const char* logName = "RollHistory.txt";
};


#endif
