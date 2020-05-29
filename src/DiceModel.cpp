
#include "DiceModel.h"
#include <iostream>
#include <fstream>

/**
 * Constructs a DiceModel object and scans config.ini for the settings and rolls
 * it should begin with.
 */
DiceModel::DiceModel() {
    std::string currSection, line;
    std::ifstream file(DEFAULT_CONFIG_PATH);
    //defaults, in case not found in the file
    aces = false;
    logPath = DEFAULT_LOG_PATH;
    delayMicroSeconds = DEFAULT_DELAY;
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.rfind('[', 0) == 0) {
                currSection = line;
            } else if (lineIsKey(line, keyAces)) {
                extractValue(line, keyAces.length());
                aces = line != "0";
            } else if (lineIsKey(line, keyLogPath)) {
                extractValue(line, keyLogPath.length());
                logPath = line;
            } else if (lineIsKey(line, keyDelay)) {
                try {
                    extractValue(line, keyDelay.length());
                    delayMicroSeconds = std::stol(line);
                } catch (std::invalid_argument &e) {/*It's already a default*/}
            } else if (currSection == sectionRolls) {
                std::string value(line);
                extractKey(line);
                extractValue(value, 0);
                if (!line.empty() && !value.empty()) {
                    savedRolls.emplace(line, value);
                }
            }
        }
        file.close();
    } else {
        generateDefaultFile();
    }
}

/**
 * Extracts a value from the given line (taken from config.ini).
 *
 * @param line The line of config.ini to extract the value from, and which is 
 * modified in place and will contain the value on return.
 * @param startAt Index at which to start searching for the ini key/value delimiter
 * '='.
 */
void DiceModel::extractValue(std::string &line, int startAt) const {
    int index = line.find('=', startAt) + 1;
    if (index == std::string::npos) {
        line = "";
    } else {
        line = line.substr(index);
    }
}

/**
 * Extracts the key from the given line taken from config.ini.
 *
 * @param line The line of config.ini to extract the key from, which is modified
 * in place and serves as a return parameter.
 */
void DiceModel::extractKey(std::string &line) const{
    int index = line.find('=');
    if (index == std::string::npos) {
        line = "";
    } else {
        line = line.substr(0, index);
    }
}

/**
 * @return Value of the aces bool.
 */
bool DiceModel::isAcing() const {
    return aces;
}

/**
 * Toggles the value of the aces bool.
 */
void DiceModel::toggleAces() {
    aces = !aces;
    updateConfig(keyAces, aces ? "1" : "0", sectionSettings);
}

/**
 * @return The path to roll log, which is stored in config.ini.
 */
const std::string &DiceModel::getLogPath() const {
    return logPath;
}

/**
 * @return The delay between rolls being displayed, in microseconds.
 */
long DiceModel::getDelay() const {
    return delayMicroSeconds;
}

/**
 * Sets the delay to the new value in memory and in config.ini.
 *
 * @param delay The new delay value.
 */
void DiceModel::setDelay(const long delay) {
    this->delayMicroSeconds = delay;
    updateConfig(keyDelay, std::to_string(delay), sectionSettings);
}

/**
 * Updates a value in config.ini with the given information.
 * A section is provided to allow duplicate keys as long as they are in different
 * sections.
 *
 * @param key The key whose value to change.
 * @param value The new value for key.
 * @param section The section to look for key in.
 *
 * @return Whether or not the file operation was successful.
 */
bool DiceModel::updateConfig(const std::string &key, const std::string &value,
                        const std::string &section) {
    if (section == sectionRolls) {
        savedRolls[key] = value;
    }
    bool inTheRightSection = false, updated = false;
    std::string line;
    std::fstream original(DEFAULT_CONFIG_PATH, std::ios::out | std::ios::in);
    std::ofstream newFile(DEFAULT_TEMP_PATH, std::ios::out);
    if (original.is_open() && newFile.is_open()) {
        while (getline(original, line)) {
            if (line == section) {
                inTheRightSection = true;
                newFile << line << std::endl;
                continue;
            } if (inTheRightSection && line.rfind('[', 0) == 0) {
                break;
            } if (lineIsKey(line, key) && inTheRightSection) {
                newFile << key << '=' << value << std::endl;
                updated = true;
                inTheRightSection = false;
            } else {
                newFile << line << std::endl;
            }
        }
        if (updated) {
            std::remove(DEFAULT_CONFIG_PATH);
            std::rename(DEFAULT_TEMP_PATH, DEFAULT_CONFIG_PATH);
        } else {
            std::remove(DEFAULT_TEMP_PATH);
        }
    }
    if (original.is_open()) original.close();
    if (newFile.is_open()) newFile.close();
    return updated;
}

/**
 * Removes the line with the given key and in the given section from config.ini.
 *
 * @param key The key whose line to delete.
 * @param section The section to look for key in.
 *
 * @return Whether or not the file operation was successful.
 */
bool DiceModel::removeLineFromConfig(const std::string &key,
                                     const std::string &section) {
    if (section == sectionRolls) savedRolls.erase(key);
    bool inTheRightSection = false, removed = false;
    std::string line;
    std::fstream original(DEFAULT_CONFIG_PATH, std::ios::out | std::ios::in);
    std::ofstream newFile(DEFAULT_TEMP_PATH, std::ios::out);
    if (original.is_open() && newFile.is_open()) {
        while (getline(original, line)) {
            if (line == section) {
                inTheRightSection = true;
                newFile << line << std::endl;
                continue;
            } if (inTheRightSection && line.rfind('[', 0) == 0) {
                break;
            } else if (!lineIsKey(line, key) || !inTheRightSection) {
                newFile << line << std::endl;
            } else {
                removed = true;
                inTheRightSection = false; //have to or it'll break above
            }
        }
        if (removed) {
            std::remove(DEFAULT_CONFIG_PATH);
            std::rename(DEFAULT_TEMP_PATH, DEFAULT_CONFIG_PATH);
        } else {
            std::remove(DEFAULT_TEMP_PATH);
        }
    }
    if (original.is_open()) original.close();
    if (newFile.is_open()) newFile.close();
    return removed;
}

/**
 * Adds a line to config.ini with the given key and value and in the given section.
 *
 * @param key The key to add.
 * @param value The new value to add for key.
 * @param section The section to put the new line in.
 *
 * @return Whether or not the file operation was successful.
 */
bool DiceModel::addLineToConfig(const std::string &key, const std::string &value,
        const std::string &section) {
    if (section == sectionRolls) savedRolls.emplace(key, value);
    bool added = false;
    std::string line;
    std::fstream original(DEFAULT_CONFIG_PATH, std::ios::out | std::ios::in);
    std::ofstream newFile(DEFAULT_TEMP_PATH, std::ios::out);
    if (original.is_open() && newFile.is_open() && !configContainsKey(key)) {
        while (getline(original, line)) {
            newFile << line << std::endl;
            if (line == section) { //the line we want to modify
                newFile << key << '=' << value << std::endl;
                added = true;
            }
        }
        std::remove(DEFAULT_CONFIG_PATH);
        std::rename(DEFAULT_TEMP_PATH, DEFAULT_CONFIG_PATH);
    }
    if (original.is_open()) original.close();
    if (newFile.is_open()) newFile.close();
    return added;
}

/**
 * Attempts to check if config.ini contains the given key.
 *
 * @param key the key to check for.
 *
 * @return True if config.ini contains the key OR if it could not be opened for
 * some reason, false if the entire file was searched a the key was not found.
 */
bool DiceModel::configContainsKey(const std::string &key) const {
    std::string line;
    std::ifstream file(DEFAULT_CONFIG_PATH);
    if (file.is_open()) {
        while (getline(file, line)) {
            if (lineIsKey(line, key)) {
                file.close();
                return true;
            }
        }
        file.close();
    } else {
        return true; //file could not be opened
    }
    return false; //the key was not found
}

/**
 * Attempts to return the roll value associated with the given key.
 *
 * @param key The name of the desired roll.
 *
 * @return The roll value associated with the given key or an empty string if no
 * such key was found.
 */
std::string DiceModel::getSavedRoll(const std::string &key) {
    if (savedRolls.find(key) != savedRolls.end())
        return savedRolls[key];
    return "";
}

/**
 * Checks if the given line's key matches the given key.
 *
 * @param line The line from config.ini to check.
 * @param key The key to check for in line.
 *
 * @return True if the line's key matches key, false otherwise.
 */
bool DiceModel::lineIsKey(const std::string &line, const std::string &key) const {
    return line.length() > key.length() && line.rfind(key, 0) == 0 &&
           line[key.length()] == '=';
}

/**
 * @return A vector of strings containing all the saved roll keys.
 */
std::vector<std::string> DiceModel::getKeys() const {
    std::vector<std::string> keySet;
    for(auto &kv : savedRolls) {
        keySet.push_back(kv.first);
    }
    return keySet;
}

/**
 * Generates a config.ini file with default values.
 */
void DiceModel::generateDefaultFile() const {
    std::ofstream file(DEFAULT_CONFIG_PATH);
    file << sectionSettings << std::endl;
    file << keyAces << '=' << '0' << std::endl;
    file << keyDelay << '=' << DEFAULT_DELAY << std::endl;
    file << keyLogPath << '=' << DEFAULT_LOG_PATH << std::endl;
    file << sectionRolls << std::endl;
    file.close();
}

/**
 * @return The number of saved rolls currently stored in memory and, theoretically,
 * in config.ini.
 */
int DiceModel::getNumRolls() const {
    return savedRolls.size();
}
