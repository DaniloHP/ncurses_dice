
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
    updateConfig(keyAces, std::to_string(aces));
}

const std::string &DiceModel::getLogPath() const {
    return logPath;
}

long DiceModel::getDelayNanoSeconds() const {
    return delayNanoSeconds;
}

void DiceModel::setDelayNanoSeconds(long delay) {
    this->delayNanoSeconds = delay;
    updateConfig(keyDelay, std::to_string(delay));
}

void DiceModel::updateConfig(const std::string &key, const std::string &newValue) {
    std::string line;
    std::string tempFileName = "temp";
    std::fstream original;
    std::ofstream newFile;
    original.open(CONFIG_PATH, std::ios::out | std::ios::in);
    newFile.open(tempFileName, std::ios::out);
    if (original.is_open() && newFile.is_open()) {
        while (getline(original, line)) {
            if (line.rfind(key, 0) == 0) { //the line we want to modify
                newFile << key << '=' << newValue << std::endl;
            } else {
                newFile << line << std::endl;
            }
        }
        std::remove(CONFIG_PATH);
        std::rename(tempFileName.c_str(), CONFIG_PATH);
    }
    if (original.is_open()) original.close();
    if (newFile.is_open()) newFile.close();
}
