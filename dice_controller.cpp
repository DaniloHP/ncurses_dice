//
// Created by danilo on 5/14/20.
//

#include "dice_controller.h"
#include <random>

int dice_controller::get_roll(int die_type) {
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1, die_type);
    return dist(mt);
}

std::vector<int>& dice_controller::parse_roll(char *roll) {
    static std::vector<int> arr;
    arr.clear();
    std::string s = roll;
    int num = 0;
    for (char c : s) {
        if ((c == 'd' || c == 'D') && num == 0) {
            arr.push_back(1);
        } else if (c > 47 && c < 58) {
            num = num * 10 + (c - 48);
        } else if (num > 0) {
            arr.push_back(num);
            num = 0;
        }
    }
    if (num > 0) {
        arr.push_back(num);
    }
    return arr;
}
void dice_controller::log_rolls(const std::string& rolls) {
    time_t now = time(nullptr);
    std::string date = ctime(&now); //TODO: make this not suck
    std::string command = "echo \"" + date + rolls +"\" >> rollHistory";
    system(command.c_str());
}