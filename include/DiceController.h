#ifndef DICE_CONTROLLER_H
#define DICE_CONTROLLER_H
#include <vector>
#include <string>
#include <random>
#include "DiceRoll.h"
#include "DiceModel.h"

class DiceController {
public:
    DiceController();
//    ~DiceController();
    void logRolls(const std::string& rolls);
    void clearLog();
    bool isAcing() const;
    void toggleAces();
    std::vector<DiceRoll*> *getAllRolls(char *roll);
    long getDelayNanoSeconds() const;
    void setDelayNanoSeconds(long delay);

private:
    //vars
    DiceModel model;
    std::random_device randomDevice;
    std::mt19937 twisterEngine;
    std::string logPath;


    //functions
    int getRoll(int);
    std::vector<int> *parseRoll(char*);

};


#endif
