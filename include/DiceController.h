#ifndef DICE_CONTROLLER_H
#define DICE_CONTROLLER_H
#include <vector>
#include <string>
#include <random>
#include <regex>
#include "DiceRoll.h"
#include "DiceModel.h"

/**
 * This class mostly interacts with the model and passes requests and queries
 * along to it. It also takes care of random number generation using a Mersenne
 * twister, as well as logging the user's rolls into a file specified in config.ini
 */
class DiceController {
public:
    DiceController();
    void clearLog() const;
    [[nodiscard]] bool isAcing() const;
    void toggleAces();
    std::vector<DiceRoll> getAllRolls(const std::string_view &roll);
    [[nodiscard]] long getDelay() const;
    void setDelay(long delay);
    bool addRoll(const std::string_view &key, const std::string_view &value);
    bool removeRoll(const std::string &key);
    bool updateRoll(const std::string &key, const std::string_view &newValue);
    std::string getSavedRoll(const std::string &key);
    [[nodiscard]] std::vector<std::string> getKeys() const;
    bool savedRollExists(const std::string &key);
    static bool isValidRollVal(const std::string_view &roll) ;
    bool isValidRollName(const std::string &key);
    int getNumRolls();

private:
    //vars
    DiceModel model;
    std::mt19937 twisterEngine;
    std::string logPath;

    //functions
    int getRoll(int) noexcept;
    static std::vector<std::pair<int, int>> parseRoll(const std::string_view &s) ;
    void logRolls(const std::string &rolls) const;
};

#endif
