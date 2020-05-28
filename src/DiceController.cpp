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
 * allocated vector that would be [{2, 6}, {1, 12}, {4, 10}], or, the number
 * of times to roll the die followed by the die value.
 * 
 * @param roll: string containing the roll collected from the user elsewhere
 * 
 * @return A new-allocated vector containing pairs of ints that describe the 
 * amount of and die type of rolls to be made.
 */
std::vector<std::pair<int, int>> *DiceController::parseRoll(
        const std::string &roll) {
    auto *result = new std::vector<std::pair<int, int>>();
    int num = 0;
    std::pair<int, int> pair {-1, -1};
    for (char c : roll) {
        if ((c == 'd' || c == 'D') && num == 0) {
            pair.first = 1; //ex: "d20", with no quantifier
        } else if (c > 47 && c < 58) {
            num = num * 10 + (c - 48);
        } else if (num > 0) {
            if (pair.first < 0) {
                pair.first = num;
            } else if (pair.second < 0 ) {
                pair.second = num;
            }
            num = 0;
        } if (pair.first > 0 and pair.second > 0) {
            result->emplace_back(pair);
            pair = {-1, -1};
        }
    }
    if (num > 0) {
        pair.second = num;
        result->push_back(pair);
    }
    return result;
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
 * Given a string that looks something like "2d6 d12 4d10", this function returns
 * a new-allocated vector containing DiceRoller objects to be displayed by the
 * view.
 * Each DiceRoll object in the returned vector represents a roll, where a roll
 * is, for example, 2d6 using the above example. That example represents 3
 * different rolls, and the returned vector will be of size 3.
 *
 * @param roll The string containing the roll from user input.
 *
 * @return a new-allocated vector of DiceRoll objects.
 */
std::vector<DiceRoll> *DiceController::getAllRolls(const std::string &roll) {
    std::string totalRoll;
    bool aces = model.isAcing();
    auto *allRolls = new std::vector<DiceRoll>();
    std::vector<std::pair<int, int>> *rollNums = parseRoll(roll);
    int rollVal;
    for (std::pair<int, int> &p : *rollNums) {
        DiceRoll dr;
        dr.reps = dr.origReps = p.first;
        dr.dieType = p.second;
        totalRoll += std::to_string(dr.reps) + "d" + std::to_string(dr.dieType) + ": ";
        for (int j = 0; j < dr.reps; j++) {
            rollVal = getRoll(dr.dieType);
            dr.addRoll(rollVal);
            totalRoll += std::to_string(rollVal) + " ";
            dr.sum += rollVal;
            if (aces && rollVal == dr.dieType && dr.dieType != 1)
                dr.reps++;
        }
        allRolls->emplace_back(dr);
        totalRoll += '\n';
    }
    delete rollNums;
    logRolls(totalRoll);
    return allRolls;
}

/**
 * If "aces" are in effect, dice rolled at their max values will be rolled again.
 * For example, if 2d6 were supposed to be rolled, and one of the dice rolls a 6,
 * 3d6 will end up being rolled. Theoretically this can happen infinitely.
 * 
 * @return whether or not aces are currently in effect.
 */
bool DiceController::isAcing() const {
    return model.isAcing();
}

/**
 * Toggles the acing boolean.
 */
void DiceController::toggleAces() {
    model.toggleAces();
}

/**
 * @return The delay between each die being displayed on screen in microseconds.
 */
long DiceController::getDelay() const {
    return model.getDelay();
}

/**
 * Set the delay between each die being displayed on screen in microseconds.
 *
 * @param delay The new value for delay.
 */
void DiceController::setDelay(long delay) {
    model.setDelay(delay);
}

/**
 * Given a key and new value, attempts to update the roll with the given key to
 * the given value.
 *
 * @param key A string that should match a key in the model's saved rolls.
 * @param newValue The new roll value to be assigned to the roll
 *
 * @return true if the value was changed in the model's map, false otherwise.
 */
bool DiceController::updateRoll(const std::string &key, const std::string &newValue) {
    return model.updateConfig(key, newValue, model.sectionRolls);
}

/**
 * Given a key attempts to delete the roll with the given key.
 *
 * @param key A string that should match a key in the model's saved rolls.
 *
 * @return true if the value was deleted in the model's map, false otherwise.
 */
bool DiceController::removeRoll(const std::string &key) {
    return model.removeLineFromConfig(key, model.sectionRolls);
}

/**
 * Adds the given key and roll value as a new saved roll in the model and by
 * extension config.ini
 *
 * @param key The name or identifier of the new roll
 * @param value The actual value of the new roll
 *
 * @return Whether or not the roll was able to be added.
 */
bool DiceController::addRoll(const std::string &key, const std::string &value) {
    return model.addLineToConfig(key, value, model.sectionRolls);
}

/**
 * Returns the roll value associated with the given key
 *
 * @param key The identifier of the roll to return
 *
 * @return the roll value associated with the given key
 */
std::string DiceController::getSavedRoll(const std::string &key) {
    return model.getSavedRoll(key);
}

/**
 * @return A new-allocated vector of all the saved roll names as strings
 */
std::vector<std::string> *DiceController::getKeys() {
    return model.getKeys();
}

/**
 * Checks if a roll of the given key exists.
 *
 * @param key The name of the roll to check
 *
 * @return true if there exists a roll by that exact name, false otherwise.
 */
bool DiceController::savedRollExists(const std::string &key) {
    return !model.getSavedRoll(key).empty();
}

/**
 * Checks to see if a given roll value is valid by checking it against the regex
 * defined as a macro in this class's header.
 *
 * @param The roll to check for validity.
 *
 * @rerurn True if the roll is deemed valid, false otherwise.
 *
 */
bool DiceController::isValidRollVal(const std::string &roll) {
    return regex_match(roll, std::regex(ROLL_REGEX));
}

/**
 * Checks to see if the given roll name is valid by making sure a roll of the
 * same name does not already exist, and that the name would not be a valid roll
 * value.
 *
 * @param key The name to check for validity.
 *
 * @return True if the key is valid, false otherwise.
 */
bool DiceController::isValidRollName(const std::string &key) {
    return !savedRollExists(key) && !isValidRollVal(key) && key[0] != '\0';
}

/**
 * @return The number of saved rolls stored in the model.
 */
int DiceController::getNumRolls() {
    return model.getNumRolls();
}
