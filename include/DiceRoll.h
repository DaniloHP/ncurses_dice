#ifndef DICE_ROLL_H
#define DICE_ROLL_H
#include <vector>

/**
 * This simple struct is meant to bundle together data on a single roll. In this
 * case a roll can be defined as x amount of reps of one single die type: 4d10 is
 * one roll, and all of its information, including 4 (or more) randomly generated
 * die values, will be stored in this struct and passed to the view for displaying. 
 */
struct DiceRoll {

private:
    std::vector<int> rolls;

public:
    int sum, dieType, reps, origReps;

    /**
     * Initiates all members to 0.
     */
    DiceRoll() {
        sum = dieType = reps = origReps = 0;
    }

    /**
     * Clears the rolls vector.
     */
    ~DiceRoll() {
        rolls.clear();
    }

    /**
     * @return the number of reps minus the original number, or, the number of
     * extra rolls done as a result of aces.
     */
    int getNumAces() const {
        return reps - origReps;
    }

    /**
     * Returns the int at the given index
     *
     * @param i The index to look at
     *
     * @return The roll at the i or -1 if the index is out of bounds
     */
    int getAt(int i) const  {
        if (i < rolls.size() && i >= 0)
            return rolls.at(i);
        else
            return -1;
    }

    /**
     * Adds a roll result to the internal vector.
     *
     * @param val The roll to add.
     */
    void addRoll(int val) {
        rolls.emplace_back(val);
    }
};

#endif //NCURSES_DICEROLL_H
