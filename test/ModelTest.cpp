#include <iostream>
#include "../include/DiceModel.h"

using namespace std;
int main() {
    DiceModel model;
    cout << model.getLogPath() << endl;
    cout << model.getDelayNanoSeconds() << endl;
    model.toggleAces();
    return 0;
}