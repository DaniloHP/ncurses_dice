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
    long getDelay() const;
    void setDelay(long delay);
    bool addRoll(const std::string &key, const std::string &value);
    bool removeRoll(const std::string &key);
    bool updateRoll(const std::string &key, const std::string &newValue);
    std::string getSavedRoll(const std::string &key);
    std::vector<std::string> *getKeys();
    bool savedRollExists(const std::string &key);

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
