#ifndef DICE_MODEL_H
#define DICE_MODEL_H

#include <string>
#include <map>
#include <vector>

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
    //functions
    DiceModel();
    [[nodiscard]] bool isAcing() const;
    void toggleAces();
    [[nodiscard]] const std::string & getLogPath() const noexcept;
    [[nodiscard]] long getDelay() const noexcept;
    void setDelay(long delay);
    [[nodiscard]] bool configContainsKey(const std::string_view &key) const;
    bool addLineToConfig(const std::string_view &key, const std::string_view &value,
                         const std::string_view &section);
    bool updateConfig(const std::string &key, const std::string_view &value,
                      const std::string_view &section);
    bool removeLineFromConfig(const std::string &key, const std::string_view &section);
    std::string getSavedRoll(const std::string &key);
    [[nodiscard]] std::vector<std::string> getKeys() const;
    [[nodiscard]] int getNumRolls() const;

private:
    //vars
    bool aces;
    long delayMicroSeconds;
    std::string logPath;
    const std::string keyAces = "bAces";
    const std::string keyLogPath = "rollLogSavePath";
    const std::string keyDelay = "delayMicroSeconds";
    const std::string defaultConfigPath = "config.ini";
    std::map<std::string, std::string> savedRolls;


    //functions
    [[nodiscard]] bool lineIsKey(const std::string_view &line, const std::string_view &key) const;
    [[nodiscard]] std::string_view extractValue(const std::string_view &line, int startAt) const;
    [[nodiscard]] std::string_view extractKey(const std::string_view &line) const;
    void generateDefaultFile(unsigned int delay) const;
};

#endif
