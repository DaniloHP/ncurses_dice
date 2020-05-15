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
#define DEFAULT_TEMP_PATH "temp"
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
    bool configContainsKey(const std::string &key);
    void addLineToConfig(const std::string &key, const std::string &newValue,
                         const std::string &section);
    void updateConfig(const std::string &key, const std::string &newValue,
                      const std::string &section);
    void
    removeLineFromConfig(const std::string &key, const std::string &section);

    const std::string sectionSettings = "[settings]";
    const std::string sectionRolls = "[rolls]";

private:
    bool aces;
    long delayNanoSeconds;
    std::string logPath;
    const std::string keyAces = "bAces";
    const std::string keyLogPath = "rollLogSavePath";
    const std::string keyDelay = "delayNanoSeconds";
    std::map<std::string, std::string> savedRolls;

    bool lineIsKey(const std::string &line, const std::string &key);

};

#endif //NCURSES_DICEMODEL_H
