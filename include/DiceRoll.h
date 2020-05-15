//
// Created by danilo on 5/14/20.
//

#ifndef DICE_ROLL_H
#define DICE_ROLL_H
#include <vector>

class DiceRoll {
private:
    int sum, dieType, reps, origReps, numAces;
    std::vector<int> *rolls;

public:

    DiceRoll();

    ~DiceRoll();

    int getSum() const;

    void setSum(int nSum);

    int getDieType() const;

    void setDieType(int nDieType);

    int getReps() const;

    void setReps(int nReps);

    int getOrigReps() const;

    void setOrigReps(int nOrigReps);

    int getNumAces() const;

    int getAt(int i) const;

    void pushBack(int val);
};

#endif //NCURSES_DICEROLL_H
