#ifndef DICE_MODEL_H
#define DICE_MODEL_H

#include <string>
#include <map>

#ifdef DEBUG
#define CONFIG_PATH "../config.ini"
#else
#define CONFIG_PATH "config.ini"
#endif

#define DEFAULT_LOG_PATH "RollHistory.txt"
#define DEFAULT_DELAY 80000

class DiceModel {
public:
    DiceModel();
//    ~DiceModel();
    bool isAcing() const;
    void toggleAces();
    const std::string &getLogPath() const;
    long getDelayNanoSeconds() const;
    void setDelayNanoSeconds(long delay);

private:
    bool aces;
    long delayNanoSeconds;
    std::map<std::string, std::string> savedRolls;
    std::string logPath;
    const std::string keyAces = "bAces";
    const std::string keyLogPath = "rollLogSavePath";
    const std::string keyDelay = "delayNanoSeconds";
    void updateConfig(const std::string &key, const std::string &newValue);
};

#endif //NCURSES_DICEMODEL_H
