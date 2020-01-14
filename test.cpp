#include <cstdio>
#include <iostream>
#include <vector>
#include <regex>

using namespace std;
vector<int>& parseRoll(char* roll) {
    static vector<int> arr;
    string s = roll;
    int num = 0;
    for (char c : s) {
        if ((c == 'd' || c == 'D') && num == 0) {
            arr.push_back(1);
        } else if (c > 47 && c < 58) {
            num = num * 10 + (c - 48);
        } else if (num > 0) {
            arr.push_back(num);
            num = 0;
        }
    }
    if (num > 0) {
        arr.push_back(num);
    }
    return arr;
}

int main() {
    vector<int> vec = parseRoll("1d10 d4 d8 4d10");
    for (int &n : vec) {
        cout << n << ", ";
    }
}