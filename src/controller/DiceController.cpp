#include "DiceController.h"
#include <random>
#include <fstream>
#include <string>

/**
 * initiates the Mersenne twister object and collects the log path from the model.
 */
DiceController::DiceController() {
    std::mt19937 mt(randomDevice());
    this->twisterEngine = mt;
    logPath = model.getLogPath();
}

/**
 * Returns a pseudo-random int between 1 and the given parameter. 
 * @param dieType The upper limit (inclusive). If rolling d8, call getRoll(8).
 * 
 * @return A pseudo-random int between 1 and dieType
 */
int DiceController::getRoll(int dieType) {
    std::uniform_int_distribution<int> dist(1, dieType);
    return dist(twisterEngine);
}

/**
 * Given a roll that looks like "2d6 d12 4d10", this function returns a heap-
 * allocated vector that would be [2, 6, 1, 12, 4, 10], or, the number of times 
 * to roll the die followed by the die value.
 * 
 * @param roll: string containing the roll collected from the user elsewhere
 * 
 * @return A new-allocated vector containing ints that describe the amount of and
 * die type of rolls to be made.
 */
std::vector<int>* DiceController::parseRoll(const std::string &roll) {
    auto *arr = new std::vector<int>();
    int num = 0;
    for (char c : roll) {
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
    return arr;
}

/**
 * Appends the given string to a log file for bookkeeping.
 * 
 * @param rolls: a string of rolls to be logged to a file.
 */
void DiceController::logRolls(const std::string& rolls) {
    time_t now = time(nullptr);
    std::string date = ctime(&now);
    std::ofstream file;
    file.open(logPath, std::ios_base::app);
    file << date + rolls << std::endl;
    file.close();
    date.erase(0);
}

/**
 * Truncates the log file.
 */
void DiceController::clearLog() {
    std::ofstream file;
    file.open(logPath, std::ofstream::out | std::ofstream::trunc);
    file.close();
}

/**
 * 
 */
std::vector<DiceRoll*> *DiceController::getAllRolls(char *roll) {
    bool aces = model.isAcing();
    auto *allRolls = new std::vector<DiceRoll*>();
    std::vector<int> *rollNums = parseRoll(roll);
    int sum, origReps, dieType, reps, rollVal, i, j;
    for (i = 0; i < rollNums->size(); i += 2) {
        auto currRoll = new DiceRoll();
        sum = 0;
        dieType = rollNums->at(i + 1);
        reps = origReps = rollNums->at(i);
        currRoll->dieType = dieType;
        currRoll->origReps = origReps;
        for (j = 0; j < reps; j++) {
            rollVal = getRoll(dieType);
            currRoll->pushBack(rollVal);
            sum += rollVal;
            if (aces && rollVal == dieType && dieType != 1) reps++;
        }
        currRoll->reps = reps;
        currRoll->sum = sum;
        allRolls->push_back(currRoll);
    }
    delete rollNums;
    return allRolls;
}

bool DiceController::isAcing() const {
    return model.isAcing();
}

void DiceController::toggleAces() {
    model.toggleAces();
}

long DiceController::getDelay() const {
    return model.getDelay();
}

void DiceController::setDelay(long delay) {
    model.setDelay(delay);
}

bool DiceController::updateRoll(const std::string &key, const std::string &newValue) {
    return model.updateConfig(key, newValue, model.sectionRolls);
}

bool DiceController::removeRoll(const std::string &key) {
    return model.removeLineFromConfig(key, model.sectionRolls);
}

bool DiceController::addRoll(const std::string &key, const std::string &value) {
    return model.addLineToConfig(key, value, model.sectionRolls);
}

std::string DiceController::getSavedRoll(const std::string &key) {
    return model.getSavedRoll(key);
}

std::vector<std::string> *DiceController::getKeys() {
    return model.getKeys();
}

bool DiceController::savedRollExists(const std::string &key) {
    return !model.getSavedRoll(key).empty();
}

bool DiceController::isValidRollVal(const std::string &roll) {
    return regex_match(roll, std::regex(ROLL_REGEX));
}

bool DiceController::isValidRollName(const std::string &key) {
    return !savedRollExists(key) && !isValidRollVal(key) && key[0] != '\0';
}

int DiceController::getNumRolls() {
    return model.getNumRolls();
}
