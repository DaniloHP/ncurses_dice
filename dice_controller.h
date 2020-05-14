//
// Created by danilo on 5/14/20.
//
#ifndef DICE_CONTROLLER_H
#define DICE_CONTROLLER_H
#include <vector>
#include <string>


class dice_controller {
public:
    int get_roll(int);
    std::vector<int>& parse_roll(char*);
    void log_rolls(const std::string& rolls);
};


#endif
