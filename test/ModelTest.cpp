#include <iostream>
#include <regex>
#include "../include/DiceModel.h"

using namespace std;
int main() {
    string roll = "3d6 d4 8d10";
    smatch match;
    regex ex(R"((\d{0,2}[dD]\d+(\s|$))+)");

    cout << regex_match(roll, ex) << endl;

    return 0;
}