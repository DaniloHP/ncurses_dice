
#include "DiceModel.h"
#include <iostream>
#include <fstream>

/**
 * Constructs a DiceModel object and scans config.ini for the settings and rolls
 * it should begin with.
 */
DiceModel::DiceModel() {
    /*
     *
     *
     * TODO: MAYBE MOVE THE CONFIG.INI IO TO THE CONTROLLER
     *
     *
     */
    std::string currSection, line, *key, *value;
    std::ifstream file;
    file.open(DEFAULT_CONFIG_PATH);
    //defaults, in case not found in the file
    value = nullptr;
    aces = false;
    logPath = DEFAULT_LOG_PATH;
    delayMicroSeconds = DEFAULT_DELAY;
    if (file.is_open()) {
        while (getline(file, line)) {
            if (lineIsKey(line, keyAces)) {
                value = extractValue(line, keyAces.length());
                aces = *value != "0";
                delete value;
            } else if (line.rfind(keyLogPath, 0) == 0) {
                value = extractValue(line, keyLogPath.length());
                logPath = *value;
                delete value;
            } else if (line.rfind(keyDelay, 0) == 0) {
                try {
                    value = extractValue(line, keyDelay.length());
                    delayMicroSeconds = std::stol(*value);
                    delete value;
                } catch (std::invalid_argument &e) {
                    delete value;
                }
            } else if (line.rfind('[', 0) == 0) {
                currSection = line;
            }
            if (currSection == sectionRolls) {
                key = extractKey(line);
                value = extractValue(line, 0);
                if (key != nullptr && value != nullptr) {
                    savedRolls.emplace(*key, *value);
                }
                delete key; delete value;
            }
        }
        file.close();
    } else {
        generateDefaultFile();
    }
}

/**
 * Clears the map holding the saved rolls.
 */
DiceModel::~DiceModel() {
    savedRolls.clear();
}

/**
 * Extracts a value from the given line (taken from config.ini).
 *
 * @param line The line of config.ini to extract the value from.
 * @param startAt Index at which to start searching for the ini key/value delimiter
 * '='.
 *
 * @return a new-allocated string containing the value.
 */
std::string *DiceModel::extractValue(std::string &line, int startAt) {
    //TODO: modify line in place, make void
    int index = line.find('=', startAt) + 1;
    std::string *result;
    if (index == std::string::npos) {
        return nullptr;
    }
    result = new std::string(line.substr(index));
    return result;
}

/**
 * Extracts the key from the given line (taken from config.ini).
 *
 * @param line The line of config.ini to extract the key from.
 *
 * @return a new-allocated string containing the key.
 */
std::string *DiceModel::extractKey(std::string &line) {
    int index = line.find('=');
    std::string *result;
    if (index == std::string::npos) {
        return nullptr;
    }
    result = new std::string(line.substr(0, index));
    return result;
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
void DiceModel::setDelay(long delay) {
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
    std::fstream original;
    std::ofstream newFile;
    original.open(DEFAULT_CONFIG_PATH, std::ios::out | std::ios::in);
    newFile.open(DEFAULT_TEMP_PATH, std::ios::out);
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
    std::fstream original;
    std::ofstream newFile;
    original.open(DEFAULT_CONFIG_PATH, std::ios::out | std::ios::in);
    newFile.open(DEFAULT_TEMP_PATH, std::ios::out);
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
    std::fstream original;
    std::ofstream newFile;
    original.open(DEFAULT_CONFIG_PATH, std::ios::out | std::ios::in);
    newFile.open(DEFAULT_TEMP_PATH, std::ios::out);
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
bool DiceModel::configContainsKey(const std::string &key) {
    std::string line;
    std::ifstream file;
    file.open(DEFAULT_CONFIG_PATH);
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
bool DiceModel::lineIsKey(const std::string &line, const std::string &key) {
    return line.length() > key.length() && line.rfind(key, 0) == 0 &&
           line.at(key.length()) == '=';
}

/**
 * @return A new-allocated vector of strings containing all the saved roll keys.
 */
std::vector<std::string> *DiceModel::getKeys() {
    auto keySet = new std::vector<std::string>();
    for(auto &kv : savedRolls) {
        keySet->push_back(kv.first);
    }
    return keySet;
}

/**
 * Generates a config.ini file with default values.
 */
void DiceModel::generateDefaultFile() {
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
int DiceModel::getNumRolls() {
    return savedRolls.size();
}
