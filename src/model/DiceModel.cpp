
#include "../../include/DiceModel.h"
#include <iostream>
#include <fstream>

DiceModel::DiceModel() {
    std::string line;
    std::ifstream file;
    file.open(CONFIG_PATH);

    //defaults, in case not found in the file
    aces = false;
    logPath = DEFAULT_LOG_PATH;
    delayNanoSeconds = DEFAULT_DELAY;

    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.rfind(keyAces, 0) == 0) {
                aces = line.at(keyAces.length() + 1) == '1';
            } else if (line.rfind(keyLogPath, 0) == 0) {
                logPath = line.substr(keyLogPath.length() + 1);
            } else if (line.rfind(keyDelay, 0) == 0) {
                try { //stol is only >c++11
                    delayNanoSeconds = std::stol(line.substr(keyDelay.length() + 1));
                } catch (std::invalid_argument &e) {
                    //Already a default. Not much to do here, maybe erase it?
                }
            }
        }
        file.close();
    }
}

//DiceModel::~DiceModel() {
//    savedRolls.clear(); //double frees from either this or the controller's
//}

bool DiceModel::isAcing() const {
    return aces;
}

void DiceModel::toggleAces() {
    aces = !aces;
    updateConfig(keyAces, aces ? "1" : "0", sectionSettings);
}

const std::string &DiceModel::getLogPath() const {
    return logPath;
}

long DiceModel::getDelayNanoSeconds() const {
    return delayNanoSeconds;
}

void DiceModel::setDelayNanoSeconds(long delay) {
    this->delayNanoSeconds = delay;
    updateConfig(keyDelay, std::to_string(delay), sectionSettings);
}

void
DiceModel::updateConfig(const std::string &key, const std::string &newValue,
                        const std::string &section) {
    bool inTheRightSection = false, updated = false;
    std::string line;
    std::fstream original;
    std::ofstream newFile;
    original.open(CONFIG_PATH, std::ios::out | std::ios::in);
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
                newFile << key << '=' << newValue << std::endl;
                updated = true;
                inTheRightSection = false;
            } else {
                newFile << line << std::endl;
            }
        }
        if (updated) {
            std::remove(CONFIG_PATH);
            std::rename(DEFAULT_TEMP_PATH, CONFIG_PATH);
        } else {
            std::remove(DEFAULT_TEMP_PATH);
        }
    }
    if (original.is_open()) original.close();
    if (newFile.is_open()) newFile.close();
}

void DiceModel::removeLineFromConfig(const std::string &key,
                                     const std::string &section) {
    bool inTheRightSection = false, removed = false;
    std::string line;
    std::fstream original;
    std::ofstream newFile;
    original.open(CONFIG_PATH, std::ios::out | std::ios::in);
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
            std::remove(CONFIG_PATH);
            std::rename(DEFAULT_TEMP_PATH, CONFIG_PATH);
        } else {
            std::remove(DEFAULT_TEMP_PATH);
        }
    }
    if (original.is_open()) original.close();
    if (newFile.is_open()) newFile.close();
}

void DiceModel::addLineToConfig(const std::string &key, const std::string &newValue,
        const std::string &section) {
    std::string line;
    std::fstream original;
    std::ofstream newFile;
    original.open(CONFIG_PATH, std::ios::out | std::ios::in);
    newFile.open(DEFAULT_TEMP_PATH, std::ios::out);
    if (original.is_open() && newFile.is_open() && !configContainsKey(key)) {
        while (getline(original, line)) {
            newFile << line << std::endl;
            if (line == section) { //the line we want to modify
                newFile << key << '=' << newValue <<std::endl;
            }
        }
        std::remove(CONFIG_PATH);
        std::rename(DEFAULT_TEMP_PATH, CONFIG_PATH);
    }
    if (original.is_open()) original.close();
    if (newFile.is_open()) newFile.close();
}

bool DiceModel::configContainsKey(const std::string &key) {
    std::string line;
    std::ifstream file;
    file.open(CONFIG_PATH);
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

bool DiceModel::lineIsKey(const std::string &line, const std::string &key) {
    return line.rfind(key, 0) == 0 && line.length() > key.length() &&
           line.at(key.length()) == '=';
}
