//
// Created by danilo on 5/14/20.
//

#include "dice_controller.h"
#include <random>
#include <fstream>
#include <string>

dice_controller::dice_controller() {
    aces = true; //eventually read from a file or something
}

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

std::vector<dice_roll> *dice_controller::get_all_rolls(std::vector<int>* roll_nums) {
    auto *ALL_ROLLS = new std::vector<std::vector<int>>();
    std::string total_roll;
    int sum, total_sum, orig_reps, die_type, reps, roll_val, i, j;
    for (i = 0; i < roll_nums->size(); i += 2) {
        sum = 0;
        die_type = roll_nums->at(i + 1);
        reps = orig_reps = roll_nums->at(i);
        total_roll += std::to_string(reps) + "d" + std::to_string(die_type) + ": ";
        for (j = 0; j < reps; j++) {
            roll_val = get_roll(die_type);
            sum += roll_val;
            total_roll += std::to_string(roll_val) + " ";
            if (aces && roll_val == die_type && die_type != 1) reps++;
        }
        total_sum += sum;
        total_roll += "\n";
        std::string aces_msg = "| " + std::to_string((reps - orig_reps)) + " ace(s)";
        aces_msg.erase(0);
    }
}

bool dice_controller::is_acing() const {
    return aces;
}

void dice_controller::toggle_aces() {
    aces = !aces;
}
