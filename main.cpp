
#include <ncurses.h>
#include <regex>
#include <vector>
#include <unistd.h>
#include <ctime>
#include <random>

using namespace std;

vector<int>& parseRoll(char* roll);
int getNumLength(int n);
void logRolls(string* rolls);
void printMenu(int &highlight, int &choice, WINDOW *whatDo);
void doRolls(string& totalRoll, vector<int>& rollNums, int& lastY);
void setupInWin(bool& redo, char* roll);
WINDOW* setUpWhatDo(int& lastY);

int main() {
    initscr();
    cbreak();
    bool redo, done; done = redo = false;
    char roll[40];
    while (!done) {
        setupInWin(redo, roll);
        vector<int> rollNums = parseRoll(roll);
        string totalRoll;
        int lastY = 11;
        doRolls(totalRoll, rollNums, lastY);
        logRolls(&totalRoll);

        WINDOW *whatDo = setUpWhatDo(lastY);
        int choice, highlight = redo ? 1 : 0;
        while (true) {
            printMenu(highlight, choice, whatDo);
            choice = wgetch(whatDo); //not quite right should be from whatDo
            switch (choice) {
                case KEY_UP:
                    highlight--;
                    highlight = (highlight == -1) ? 2 : highlight;
                    break;
                case KEY_DOWN:
                    highlight++;
                    highlight = (highlight == 3) ? 0 : highlight;
                    break;
                default:
                    break;
            }
            if (choice == 10) {
                redo = false;
                rollNums.clear();
                if (highlight == 2) { //exit
                    done = true;
                } else if (highlight == 1) { //redo the roll
                    redo = true;
                }
                clear();
                refresh();
                break;
            }
        }
    }
    endwin();
    return 0;
}

void setupInWin(bool &redo, char *roll) {
    int xMax = getmaxx(stdscr);
    WINDOW *inputWin = newwin(4, xMax - 12, 2, 5);
    box(inputWin, 0, 0);
    refresh();
    mvwprintw(inputWin, 1, 1, "Enter your roll:");
    if (redo) {
        mvwprintw(inputWin, 2, 1, roll);
    } else {
        echo();
        curs_set(1);
        mvwgetstr(inputWin, 2, 1, roll);
    }
    wrefresh(inputWin);
    refresh();
    curs_set(0);
    noecho();
}

vector<int> &parseRoll(char *roll) {
    static vector<int> arr;
    arr.clear();
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

void doRolls(string &totalRoll, vector<int> &rollNums, int &lastY) {
    for (int i = 0; i < rollNums.size(); i += 2) {
        int dieType = rollNums.at(i + 1), numLen = getNumLength(dieType);
        int reps = rollNums.at(i);
        totalRoll += to_string(reps) + "d" + to_string(dieType) + ": ";
        lastY = 7 + (i * 2);
        int j;
        for (j = 0; j < reps; j++) {
            //usleep(80000);
            WINDOW *die = newwin(3, numLen + 2, lastY, 5 + (j * (numLen + 3)));
            box(die, 0, 0);
            //state of the art random int generator
            random_device rd;
            mt19937 mt(rd());
            uniform_int_distribution<int> dist(1, dieType);
            int rollVal = dist(mt);

            totalRoll += to_string(rollVal) + " ";
            mvwprintw(die, 1, 1, "%d", rollVal);
            wrefresh(die);
        }
        totalRoll += "\n";
        mvprintw(lastY + 1,  (7 + (j * (numLen + 3))), "<-- %dd%d", reps,
                 dieType);
        refresh();
    }
}

void logRolls(string* rolls) {
    time_t now = time(nullptr);
    string date = ctime(&now);
    string command = "echo \"" + date + *rolls +"\" >> rollHistory";
    system(command.c_str());
}

WINDOW *setUpWhatDo(int &lastY) {
    WINDOW* whatDo = newwin(6, 40, lastY + 4, 5);
    keypad(whatDo, true);
    box(whatDo, 0, 0);
    wattron(whatDo, A_BOLD);
    mvwprintw(whatDo, 1, 1, "What do?");
    wattroff(whatDo, A_BOLD);
    wrefresh(whatDo);
    return whatDo;
}

void printMenu(int &highlight, int &choice, WINDOW *whatDo) {
    string choices[] = {"Roll again", "Reroll this roll", "Exit"};
    for (int i = 0; i < 3; i++) {
        if (i == highlight) {
            wattron(whatDo, A_REVERSE);
        }
        mvwprintw(whatDo, i + 2, 1, choices[i].c_str());
        wrefresh(whatDo);
        wattroff(whatDo, A_REVERSE);
    }
}

int getNumLength(int n) {
    if (n == 0) {
        return 0;
    } else {
        return 1 + getNumLength(n / 10);
    }
}
