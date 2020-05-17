#ifndef DICE_MODEL_H
#define DICE_MODEL_H

#include <string>
#include <map>
#include <vector>

#ifdef DEBUG
    #define CONFIG_PATH "../config.ini"
#else
    #define CONFIG_PATH "config.ini"
#endif

#define DEFAULT_LOG_PATH "RollHistory.txt"
#define DEFAULT_TEMP_PATH "temp"
#define DEFAULT_DELAY 80000

class DiceModel {
public:
    //vars
    const std::string sectionSettings = "[settings]";
    const std::string sectionRolls = "[rolls]";

    //functions
    DiceModel();
//    ~DiceModel();
    bool isAcing() const;
    void toggleAces();
    const std::string &getLogPath() const;
    long getDelay() const;
    void setDelay(long delay);
    bool configContainsKey(const std::string &key);
    bool addLineToConfig(const std::string &key, const std::string &value,
                         const std::string &section);
    bool updateConfig(const std::string &key, const std::string &value,
                      const std::string &section);
    bool removeLineFromConfig(const std::string &key, const std::string &section);
    std::string getSavedRoll(const std::string &key);
    std::vector<std::string> *getKeys();

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
    bool lineIsKey(const std::string &line, const std::string &key);
    std::string *extractValue(std::string &line, int startAt);
    std::string *extractKey(std::string &line);
};

#endif //guard
