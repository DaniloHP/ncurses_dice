#ifndef DICE_ROLL_H
#define DICE_ROLL_H
#include <vector>

/**
 * This simple struct is meant to bundle together data on a single roll. In this
 * case a roll can be defined as x amount of reps of one single die type: 4d10 is
 * one roll, and all of its inforation, including 4 (or more) randomly generated
 * die values, will be stored in this struct and passed to the view for displaying. 
 */
struct DiceRoll {

private:
    std::vector<int> *rolls;

public:
    int sum, dieType, reps, origReps;

    DiceRoll() {
        sum = dieType = reps = origReps;
        rolls = new std::vector<int>();
    }

    ~DiceRoll() {
        rolls->clear();
        delete rolls;
    }

    int getNumAces() const {
        return reps - origReps;
    }

    int getAt(int i) const  {
        if (i < rolls->size())
            return rolls->at(i);
        else
            return -1;
    }

    void pushBack(int val) {
        rolls->emplace_back(val);
    }
};

#endif //NCURSES_DICEROLL_H
