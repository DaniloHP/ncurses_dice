#include <ncurses.h>
#include <regex>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include "../../include/DiceController.h"

#define ENTER 10
#define BUF_SIZE 40
#define ROLL_REGEX R"((\d*[dD]\d+(\s+|$))+)"

void printMenu(int &highlight, int &choice, WINDOW *win, const char **choices,
        int size, short offset);
void printRolls(std::vector<DiceRoll*> *allRolls, int &lastY, DiceController &controller);
void setupInputWin(WINDOW *&inWin, bool redo, char *roll,
                   DiceController &controller);
void setUpWhatDo(int lastY, WINDOW *&whatDo);
void handleSettings(int lastY, DiceController &controller);
void displayInputWin(WINDOW *inWin, char *roll);
int  getNumLength(int n);
void handleChangeRollDelay(WINDOW *setWin, DiceController &controller);
void handleClearLog(WINDOW *setWin, DiceController &controller);
void handleToggleAces(WINDOW *setWin, DiceController &controller);
void handleSavedRolls(int lastY, DiceController &controller);
void handleRemoveRoll(WINDOW *rollsWin, DiceController &controller);
void handleNewOrUpdateRoll(int lastY, DiceController &controller, bool update);
void indicateError(WINDOW *const &window, int problemY, int problemX,
                   const char *msg, int originalY, int originalX);

void printSavedRolls(int lastY, DiceController &controller);

int main() {
    WINDOW *whatDo = nullptr, *inputWin = nullptr, *mainScreen = initscr();
    DiceController controller;
    char roll[BUF_SIZE];
    int lastY, choice, highlight;
    std::vector<int> rollNums;
    std::vector<DiceRoll*> *allRolls;
    const char *whatDoChoices[] = {
            (const char*)"Roll again", (const char*)"Reroll this roll",
            (const char*)"Saved rolls", (const char*)"Settings",
            (const char*)"Exit"};
    bool redo, done, enteredSubmenu;
    done = redo = false;
    cbreak();
    while (!done) {
        setupInputWin(inputWin, redo, roll, controller); //this collects input
        lastY = 11;
        allRolls = controller.getAllRolls(roll);
        printRolls(allRolls, lastY, controller);
        setUpWhatDo(lastY, whatDo);
        highlight = redo ? 1 : 0; //if last roll was redo, start with reroll HL'd
        while (true) {
            redo = enteredSubmenu = false;
            printMenu(highlight, choice, whatDo, whatDoChoices, 5, 2);
            if (choice == ENTER) { //perhaps a map from index to function pointer for handling?
                if (highlight == 4) { //exit
                    done = true;
                } else if (highlight == 3) { //settings
                    handleSettings(lastY, controller);
                    enteredSubmenu = true;
                } else if (highlight == 2) {
                    handleSavedRolls(lastY, controller);
                    enteredSubmenu = true;
                } else if (highlight == 1) { //redo the roll
                    redo = true;
                }
                if (!enteredSubmenu) {
                    clear();
                    refresh();
                    displayInputWin(inputWin, roll);
                    wrefresh(inputWin);
                    break;
                }
            }
        }
    }
    delwin(inputWin);
    delwin(whatDo);
    delwin(mainScreen);
    endwin();
    rollNums.clear();
    return 0;
}

void setupInputWin(WINDOW *&inWin, bool redo, char *roll,
        DiceController &controller) {
    int xMax = getmaxx(stdscr);
    std::string rollValue;
    if (inWin == nullptr) {
        inWin = newwin(4, xMax - 12, 2, 5);
    }
    wclear(inWin);
    box(inWin, 0, 0);
    mvwprintw(inWin, 1, 1, "Enter your roll:");
    if (!redo) {
        echo();
        curs_set(1);
        mvwgetnstr(inWin, 2, 1, roll, BUF_SIZE);
        rollValue = controller.getSavedRoll(roll);
        while (!regex_match(roll, std::regex(ROLL_REGEX)) && rollValue.empty()) {
            indicateError(inWin, 1, 1, "Invalid roll format", 2, 1);
            box(inWin, 0, 0);
            mvwprintw(inWin, 1, 1, "Enter your roll:");
            mvwgetnstr(inWin, 2, 1, roll, BUF_SIZE);
            rollValue = controller.getSavedRoll(roll);
        }
        if (!rollValue.empty()) {
            strncpy(roll, rollValue.c_str(), BUF_SIZE);
            redo = true;
            mvwprintw(inWin, 2, strlen(roll) + 2, "(%s)", roll);
            wrefresh(inWin);
        }
    } if (redo) {
        wmove(inWin, 2, 1);
        wclrtoeol(inWin);
        mvwprintw(inWin, 2, 1, roll);
    }
    curs_set(0);
    noecho();

}

void indicateError(WINDOW *const &window, int problemY, int problemX,
                   const char *msg, int originalY, int originalX) {
    wmove(window, problemY, problemX);
    curs_set(0);
    wclrtoeol(window);
    wattron(window, A_BOLD);
    box(window, 0, 0);
    mvwprintw(window, problemY, problemX, msg);
    wattroff(window, A_BOLD);
    wrefresh(window);
    usleep(1500000);
    box(window, 0, 0);
    wmove(window, problemY, problemX);
    wclrtoeol(window);
    wmove(window, originalY, originalX);
    wclrtoeol(window);
    wrefresh(window);
    curs_set(1);
}

void displayInputWin(WINDOW *inWin, char *roll) {
    box(inWin, 0, 0);
    mvwprintw(inWin, 1, 1, "Enter your roll:");
    mvwprintw(inWin, 2, 1, roll);
    curs_set(0);
    noecho();
} //wrefresh does not work in here, maybe because double pointer or something?

void setUpWhatDo(int lastY, WINDOW *&whatDo) {
    int numSettings = 5;
    if (whatDo == nullptr) {
        whatDo = newwin(numSettings + 3, 40, lastY + 4, 5);
    } else {
        mvwin(whatDo, lastY + 4, 5);
    }
    keypad(whatDo, true);
    box(whatDo, 0, 0);
    wattron(whatDo, A_BOLD);
    mvwprintw(whatDo, 1, 1, "What do?");
    wattroff(whatDo, A_BOLD);
    wrefresh(whatDo);
}

void printMenu(int& highlight, int& choice, WINDOW* win, const char **choices,
               int size, short offset) {
    for (int i = 0; i < size; i++) {
        if (i == highlight) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, i + offset, 1, choices[i]);
        wrefresh(win);
        wattroff(win, A_REVERSE);
    }
    choice = wgetch(win);
    switch (choice) {
        case KEY_UP:
            highlight--;
            highlight = (highlight == -1) ? size - 1 : highlight;
            break;
        case KEY_DOWN:
            highlight++;
            highlight = (highlight == size) ? 0 : highlight;
            break;
        default:
            break;
    }
}

void handleSavedRolls(int lastY, DiceController &controller) {
    int highlight, choice;
    highlight = choice = 0;
    const char *choices[] = {
            (const char *)"Add a new roll", (const char *)"Update a roll",
            (const char *)"List saved rolls", (const char *)"Remove a roll",
            (const char *)"Exit saved rolls"};
    int numChoices = 5;
    WINDOW* rollsWin = newwin(numChoices + 2, 46, lastY + 4, 46);
    keypad(rollsWin, true);
    box(rollsWin, 0, 0);
    while (true) {
        wclear(rollsWin);
        box(rollsWin, 0, 0);
        printMenu(highlight, choice, rollsWin, choices, numChoices, 1);
        if (choice == 10) { //use pressed enter
            if (highlight == 0) { //new roll
                handleNewOrUpdateRoll(lastY + 12, controller, false);
            } else if (highlight == 1) { //update roll
                handleNewOrUpdateRoll(lastY + 12, controller, true);
            } else if (highlight == 2) { //list rolls
                printSavedRolls(lastY + 12, controller);
            } else if (highlight == 3) { //remove roll
                handleRemoveRoll(rollsWin, controller);
            } else if (highlight == numChoices - 1) { //exit saved rolls
                break;
            }
            curs_set(0);
            noecho();
        }
    }
    wclear(rollsWin);
    wrefresh(rollsWin);
    delwin(rollsWin);
}

void printSavedRolls(int lastY, DiceController &controller) {
    std::vector<std::string> *keys = controller.getKeys();
    int i = 1, numRolls = keys->size(), choice = 0;
    WINDOW *rollsListWin = newwin(numRolls + 3, 64, lastY, 5);
    box(rollsListWin, 0, 0);
    for (const auto &roll : *keys) {
        wattron(rollsListWin, A_BOLD);
        mvwprintw(rollsListWin, i++, 1, "%s: ", roll.c_str());
        wattroff(rollsListWin, A_BOLD);
        wprintw(rollsListWin, controller.getSavedRoll(roll).c_str());
    }
    keys->clear();
    delete keys;
    wattron(rollsListWin, A_REVERSE);
    mvwprintw(rollsListWin, numRolls + 1, 1, "Return");
    wattroff(rollsListWin, A_REVERSE);
    wrefresh(rollsListWin);
    keypad(rollsListWin, true);
    curs_set(0);
    while (choice != ENTER) {
        choice = wgetch(rollsListWin);
    }
    wclear(rollsListWin);
    wrefresh(rollsListWin);
    delwin(rollsListWin);
}

void collectSavedRollInput(char *name, char *value, int lastY) {
    int xMax = getmaxx(stdscr);
    WINDOW *savedRollInputWin = newwin(4, xMax - 12, lastY, 5);
    box(savedRollInputWin, 0, 0);
    curs_set(1);
    echo();
    mvwprintw(savedRollInputWin, 1, 1, "Enter the name of the roll: ");
    keypad(savedRollInputWin, true);
    wrefresh(savedRollInputWin);
    mvwgetnstr(savedRollInputWin, 1, 29, name, 15);
    mvwprintw(savedRollInputWin, 2, 1, "Enter the roll value: ");
    mvwgetnstr(savedRollInputWin, 2, 23, value, BUF_SIZE);
    while (!regex_match(value, std::regex(ROLL_REGEX))) {
        indicateError(savedRollInputWin, 2, 1, "Invalid roll format", 2, 1);
        mvwprintw(savedRollInputWin, 1, 1, "Enter the name of the roll: %s", name);
        mvwprintw(savedRollInputWin, 2, 1, "Enter the roll value: ");
        mvwgetnstr(savedRollInputWin, 2, 23, value, BUF_SIZE);
    }
    wclear(savedRollInputWin);
    wrefresh(savedRollInputWin);
    delwin(savedRollInputWin);
}

void handleNewOrUpdateRoll(int lastY, DiceController &controller, bool update) {
    char rollName[16];
    char value[BUF_SIZE];
    collectSavedRollInput(rollName, value, lastY);
    std::string name = rollName, val = value;
    if (update) {
        controller.updateRoll(name, val);
    } else {
        controller.addRoll(name, val);
    }
}

void printSavedRollsAsMenu(WINDOW *win, std::vector<std::string> &keys, int &highlight,
                           int &choice, DiceController controller) {
    //eventually want to do roll deletion, updating and maybe even adding with this
    for (int i = 0; i < keys.size(); i++) {
        if (i == highlight) {
            wattron(win, A_REVERSE);
        }
        wattron(win, A_BOLD);
        mvwprintw(win, i, 1, "%s: ", keys[i].c_str());
        wattroff(win, A_BOLD);
        wprintw(win, controller.getSavedRoll(keys[i]).c_str());
        wrefresh(win);
        wattroff(win, A_REVERSE);
    }
    choice = wgetch(win);
    switch (choice) {
        case KEY_UP:
            highlight--;
            highlight = (highlight == -1) ? keys.size() - 1 : highlight;
            break;
        case KEY_DOWN:
            highlight++;
            highlight = (highlight == keys.size()) ? 0 : highlight;
            break;
        default:
            break;
    }
}

void handleRemoveRoll(WINDOW *rollsWin, DiceController &controller) {
    int row = 4;
    echo();
    char rollToRm[16];
    wmove(rollsWin, row, 1);
    wclrtoeol(rollsWin);
    mvwprintw(rollsWin, row, 1, "Name of the roll to remove: ");
    box(rollsWin, 0, 0);
    wrefresh(rollsWin);
    curs_set(1);
    keypad(rollsWin, true);
    mvwgetnstr(rollsWin, row, 29, rollToRm, 15);
    std::string key = rollToRm;
    if (!controller.removeRoll(key)) {
        indicateError(rollsWin, row, 1, "Roll not found", row, 1);
    }
}

void handleSettings(int lastY, DiceController &controller) {
    int highlight, choice;
    highlight = choice = 0;
    const char *choices[] = {
            (const char *)"Set roll delay", (const char *)"Clear roll log",
            (const char *)"Toggle aces", (const char *)"Exit settings"};
    int numSettings = 4;
    WINDOW* setWin = newwin(numSettings + 2, 40, lastY + 4, 46);
    keypad(setWin, true);
    box(setWin, 0, 0);
    while (true) {
        wclear(setWin);
        box(setWin, 0, 0);
        printMenu(highlight, choice, setWin, choices, numSettings, 1);
        if (choice == 10) { //use pressed enter
            if (highlight == 0) { //change roll delay
                handleChangeRollDelay(setWin, controller);
            } else if (highlight == 1) { //clear log
                handleClearLog(setWin, controller);
            } else if (highlight == 2) { //toggle aces
                handleToggleAces(setWin, controller);
            } else if (highlight == numSettings - 1) { //exit settings
                break;
            }
            curs_set(0);
            noecho();
        }
    }
    wclear(setWin);
    wrefresh(setWin);
    delwin(setWin);
}

void handleChangeRollDelay(WINDOW *setWin, DiceController &controller) {
    echo();
    curs_set(1);
    char newVal[10];
    wmove(setWin, 1, 1);
    wclrtoeol(setWin);
    box(setWin, 0, 0);
    mvwprintw(setWin, 1, 1, "Value (milliseconds):");
    box(setWin, 0, 0);
    mvwgetnstr(setWin, 1, 23, newVal, 10);
    try {
        long millis = std::stol(newVal);
        if (millis >= 0) {
            controller.setDelay(millis * 1000);
        } else {
            throw std::invalid_argument("No values less than 0.");
        }
    } catch (std::invalid_argument &e) {
        indicateError(setWin, 1, 1, "Invalid input",1, 1);
    }
}

void handleClearLog(WINDOW *setWin, DiceController &controller) {
    mvwprintw(setWin, 2, 1, "Really clear log? y/[n]");
    char real[3];
    echo();
    curs_set(1);
    mvwgetnstr(setWin, 2, 26, real, 3);
    if (strcmp(real, "y") == 0 || strcmp(real, "Y") == 0) {
        controller.clearLog();
    }
}

void handleToggleAces(WINDOW *setWin, DiceController &controller) {
    controller.toggleAces();
    wmove(setWin, 3, 1);
    wclrtoeol(setWin);
    box(setWin, 0, 0);
    mvwprintw(setWin, 3, 1, "Aces now %s", (controller.isAcing()) ? "on" : "off");
    box(setWin, 0, 0);
    wrefresh(setWin);
    usleep(1500000);
}

int getNumLength(int n) {
    if (n == 0) {
        return 0;
    } else {
        return 1 + getNumLength(n / 10);
    }
}

void printRolls(std::vector<DiceRoll*> *allRolls, int &lastY, DiceController &controller) {
    int sum, dieType, reps, numLen, j, k, xMax = getmaxx(stdscr), totalSum = 0, i = 0;
    int rowsOverflowed = 0;
    WINDOW *die, *statsWin, *totalWin;
    std::string totalRoll, acesMsg;
    for (const DiceRoll *dr : *allRolls) {
        sum = dr->sum;
        dieType = dr->dieType;
        numLen = getNumLength(dieType);
        reps = dr->reps;
        totalRoll += std::to_string(reps) + "d" + std::to_string(dieType) + ": ";
        lastY = 7 + ((rowsOverflowed + i++) * 3);
        //k is the real iterator that checks for reps, j is for visual purposes
        for (k = 0, j = 0; k < reps; j++, k++) {
            usleep(controller.getDelay());
            if (5 + (j * (numLen + 3)) > xMax - 12) {
 //left padding ^    ^^^^^^^^^^^^^^^^ width of all the rolls in the row
                j = 0, lastY += 3, rowsOverflowed++;
            }
            die = newwin(3, numLen + 2, lastY, 5 + (j * (numLen + 3)));
            box(die, 0, 0);
            totalRoll += std::to_string(dr->getAt(k)) + " ";
            mvwprintw(die, 1, 1, "%d", dr->getAt(k));
            wrefresh(die); //dr->getAt(k) is the actual value of the roll
            delwin(die);
        }
        totalSum += sum;
        totalRoll += '\n';
        statsWin = newwin(2, 32, lastY, (7 + (j * (numLen + 3))));
        acesMsg = "| " + std::to_string(dr->getNumAces()) + " ace(s)";
        mvwprintw(statsWin, 0, 0, "Sum: %d %s", sum, (controller.isAcing() ?
                                                      acesMsg.c_str() : ""));
        mvwprintw(statsWin, 1, 0, "<-- %dd%d", reps, dieType);
        wrefresh(statsWin);
        delwin(statsWin);
        delete dr;
    }
    if (totalSum != sum) {
        totalWin = newwin(1, 32, lastY + 3, 5);
        mvwprintw(totalWin, 0, 0, "Sum of all rolls: %d", totalSum);
        wrefresh(totalWin);
        delwin(totalWin);
    }
    allRolls->clear();
    delete allRolls;
    controller.logRolls(totalRoll);
}
