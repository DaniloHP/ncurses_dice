//
// Created by danilo on 5/14/20.
//

#ifndef DICE_ROLL_H
#define DICE_ROLL_H
#include <vector>

class DiceRoll {
private:
    int sum, dieType, reps, origReps, numAces;
    std::vector<int> dice;

public:

    int getSum() const;

    void setSum(int nSum);

    int getDieType() const;

    void setDieType(int nDieType);

    int getReps() const;

    void setReps(int nReps);

    int getOrigReps() const;

    void setOrigReps(int nOrigReps);

    int getNumAces() const;

    void setNumAces(int nNumAces);

    const std::vector<int> &getDice() const;

    void setDice(const std::vector<int> &nDice);
};

#endif //NCURSES_DICEROLL_H
