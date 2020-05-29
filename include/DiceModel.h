#ifndef DICE_MODEL_H
#define DICE_MODEL_H

#include <string>
#include <map>
#include <vector>

#ifdef DEBUG
    #define DEFAULT_CONFIG_PATH "../config.ini"
#else
    #define DEFAULT_CONFIG_PATH "config.ini"
#endif

#define ROLL_VAL_MAX 40
#define ROLL_NAME_MAX 16
#define ROLL_TOTAL_MAX ((ROLL_VAL_MAX) + (ROLL_NAME_MAX))

#define DEFAULT_LOG_PATH "RollHistory.txt"
#define DEFAULT_TEMP_PATH "temp"
#define DEFAULT_DELAY 80000

/**
 * This class represents the program's state and enables the modification and
 * querying of that state. 
 * It also takes care of file i/o with the config file and saved rolls. Saved
 * rolls are key,value pairs that are stored both in memory in a map and in
 * config.ini. These are like aliases that can be used for rolls performed
 * frequently.
 */
class DiceModel {
public:
    //vars
    const std::string sectionSettings = "[settings]";
    const std::string sectionRolls = "[rolls]";

    //functions
    DiceModel();
    bool isAcing() const;
    void toggleAces();
    const std::string &getLogPath() const;
    long getDelay() const;
    void setDelay(long delay);
    bool configContainsKey(const std::string &key) const;
    bool addLineToConfig(const std::string &key, const std::string &value,
                         const std::string &section);
    bool updateConfig(const std::string &key, const std::string &value,
                      const std::string &section);
    bool removeLineFromConfig(const std::string &key, const std::string &section);
    std::string getSavedRoll(const std::string &key);
    std::vector<std::string> getKeys() const;
    int getNumRolls() const;

private:
    //vars
    bool aces;
    long delayMicroSeconds;
    std::string logPath;
    const std::string keyAces = "bAces";
    const std::string keyLogPath = "rollLogSavePath";
    const std::string keyDelay = "delayMicroSeconds";
    std::map<std::string, std::string> savedRolls;

    //functions
    bool lineIsKey(const std::string &line, const std::string &key) const;
    std::string extractValue(const std::string &line, int startAt) const;
    std::string extractKey(const std::string &line) const;
    void generateDefaultFile() const;
};

#endif
