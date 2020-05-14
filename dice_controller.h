//
// Created by danilo on 5/14/20.
//
#ifndef DICE_CONTROLLER_H
#define DICE_CONTROLLER_H
#include <vector>
#include <string>


class dice_roll {
private:
    int sum, die_type, reps, orig_reps, num_aces;
    std::vector<int> dice;
};

class dice_controller {
public:
    dice_controller();
    int get_roll(int);
    std::vector<int> *parse_roll(char*);
    void log_rolls(const std::string& rolls);
    void clear_log();
    bool is_acing() const;
    void toggle_aces();
    std::vector<dice_roll> *get_all_rolls(std::vector<int>*);
    
private:
    bool aces;

private:
    const char* log_name = "roll_history.txt";
};


#endif
