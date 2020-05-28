#ifndef DICE_CONTROLLER_H
#define DICE_CONTROLLER_H
#include <vector>
#include <string>
#include <random>
#include <regex>
#include "DiceRoll.h"
#include "DiceModel.h"

#define ROLL_REGEX R"(((|[1-9]|\d{2,})[dD]([1-9]|\d{2,})(\s+|$))+)"
/**
 * This class mostly interacts with the model and passes requests and queries
 * along to it. It also takes care of random number generation using a Mersenne
 * twister, as well as logging the user's rolls into a file specified in config.ini
 */
class DiceController {
public:
    DiceController();
    void logRolls(const std::string& rolls);
    void clearLog();
    bool isAcing() const;
    void toggleAces();
    std::vector<DiceRoll> *getAllRolls(const std::string &roll);
    long getDelay() const;
    void setDelay(long delay);
    bool addRoll(const std::string &key, const std::string &value);
    bool removeRoll(const std::string &key);
    bool updateRoll(const std::string &key, const std::string &newValue);
    std::string getSavedRoll(const std::string &key);
    std::vector<std::string> *getKeys();
    bool savedRollExists(const std::string &key);
    static bool isValidRollVal(const std::string &roll);
    bool isValidRollName(const std::string &key);
    int getNumRolls();

private:
    //vars
    DiceModel model;
    std::random_device randomDevice;
    std::mt19937 twisterEngine;
    std::string logPath;

    //functions
    int getRoll(int);
    static std::vector<unsigned int> *parseRoll(const std::string &s);

};

#endif
