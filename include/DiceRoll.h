#ifndef DICE_ROLL_H
#define DICE_ROLL_H
#include <vector>

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
        rolls->push_back(val);
    }
};

#endif //NCURSES_DICEROLL_H
