//
// Created by danilo on 5/14/20.
//

#include "dice_controller.h"
#include <random>
#include <fstream>

int dice_controller::get_roll(int die_type) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1, die_type);
    return dist(mt);
}

std::vector<int>* dice_controller::parse_roll(char *roll) {
    auto *arr = new std::vector<int>();
    std::string s = roll;
    int num = 0;
    for (char c : s) {
        if ((c == 'd' || c == 'D') && num == 0) {
            arr->push_back(1);
        } else if (c > 47 && c < 58) {
            num = num * 10 + (c - 48);
        } else if (num > 0) {
            arr->push_back(num);
            num = 0;
        }
    }
    if (num > 0) {
        arr->push_back(num);
    }
    s.erase(0);
    return arr;
}

void dice_controller::log_rolls(const std::string& rolls) {
    time_t now = time(nullptr);
    std::string date = ctime(&now);
    std::ofstream file;
    file.open(log_name, std::ios_base::app);
    file << date + rolls << std::endl;
    file.close();
    date.erase(0);
}

void dice_controller::clear_log() {
    std::ofstream file;
    file.open(log_name, std::ofstream::out | std::ofstream::trunc);
    file.close();
}