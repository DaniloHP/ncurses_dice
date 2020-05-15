#include <iostream>
#include "../include/DiceModel.h"

using namespace std;
int main() {
    DiceModel model;
    model.updateConfig("bAces", "0442", model.sectionRolls);
    return 0;
}