#include <ncurses.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include "../../include/DiceController.h"

#define ENTER 10
// ^ ncurses.h's KEY_ENTER is not 10 for some reason

//print all the given rolls
void printRolls(std::vector<DiceRoll *> *allRolls, DiceController &controller);

//general menu printing
void printVMenu(int &highlight, int &input, WINDOW *win, const char **choices,
                int size, short offset);
void printHMenu(int& highlight, int& input, WINDOW* win, const char **choices,
                int size, short offset);
void boundsCheckVMenu(int numChoices, int &highlight, int input);
void boundsCheckHMenu(int numChoices, int &highlight, int input);

//specific window setup
void setupInputWin(WINDOW *&inWin, bool redo, char *roll,
                   DiceController &controller);
void setUpWhatDo(WINDOW *&whatDo);
void displayInputWin(WINDOW *inWin, char *roll);

//settings and its submenus
void handleSettings(DiceController &controller);
void changeRollDelay(WINDOW *setWin, DiceController &controller);
void clearRollLog(WINDOW *setWin, DiceController &controller);
void toggleAces(WINDOW *setWin, DiceController &controller);

//saved rolls and its submenus
void handleSavedRolls(DiceController &controller);
void printSavedRollMenu(WINDOW *rollsWin, std::vector<std::string> &choices,
                        int &highlight, int &input, DiceController &controller);
void rollRename(WINDOW *optWin, int &highlight, DiceController &controller,
                std::vector<std::string> *choices);
void rollRedefine(WINDOW *rollsWin, int highlight, DiceController &controller);
void newRoll(WINDOW *rollsWin, int &highlight, DiceController &controller,
             std::vector<std::string> *choices);
void rollDelete(WINDOW *rollsWin, int highlight, std::vector<std::string> *choices,
           DiceController &controller);

//miscellaneous
int  getNumLength(int n);
void indicateError(WINDOW *const &window, int problemY, int problemX, int originalY,
              int originalX, const char *msg);

int lastY = 11;

int main() {
    WINDOW *whatDo = nullptr, *inputWin = nullptr, *mainScreen = initscr();
    DiceController controller;
    char roll[ROLL_VAL_MAX];
    int input, highlight;
    std::vector<int> rollNums;
    std::vector<DiceRoll*> *allRolls;
    const char *whatDoChoices[] = { "Roll again", "Reroll this roll",
            "Saved rolls", "Settings", "Exit" };
    bool redo, done, enteredSubmenu;
    done = redo = false;
    cbreak();
    while (!done) { //enter a roll or a saved roll name at the top of this loop
        setupInputWin(inputWin, redo, roll, controller); //collects input
        lastY = 11;
        allRolls = controller.getAllRolls(roll);
        printRolls(allRolls, controller);
        setUpWhatDo(whatDo);
        highlight = redo ? 1 : 0; //if last roll was redo, start with reroll HL'd
        while (true) { //loop for the "what do" menu
            redo = enteredSubmenu = false;
            printVMenu(highlight, input, whatDo, whatDoChoices, 5, 2);
            if (input == ENTER) { //perhaps a map from index to function pointer for handling?
                if (highlight == 4) { //exit
                    done = true;
                } else if (highlight == 3) { //settings
                    handleSettings(controller);
                    enteredSubmenu = true;
                } else if (highlight == 2) {
                    handleSavedRolls(controller);
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

void printRolls(std::vector<DiceRoll *> *allRolls, DiceController &controller) {
    int j, k, numLen, xMax = getmaxx(stdscr), totalSum = 0, i = 0, wraps = 0;
    WINDOW *die, *statsWin, *totalWin;
    std::string totalRoll, acesMsg;
    for (const DiceRoll *dr : *allRolls) {
        numLen = getNumLength(dr->dieType);
        totalRoll += std::to_string(dr->reps) + "d" + std::to_string(dr->dieType) + ": ";
        lastY = 7 + (3 * (wraps + i++));
        //k is the real iterator that checks for reps, j is for visual purposes
        for (j = 0, k = 0; k < dr->reps; j++, k++) {
            usleep(controller.getDelay());
            if (5 + (j * (numLen + 3)) > xMax - 12) { //wrap rolls onto new line
                //left padding ^    ^^^^^^^^^^^^^^^^ width of all the rolls in the row
                j = 0, lastY += 3, wraps++;
            }
            die = newwin(3, numLen + 2, lastY, 5 + (j * (numLen + 3)));
            box(die, 0, 0);
            totalRoll += std::to_string(dr->getAt(k)) + " ";
            mvwprintw(die, 1, 1, "%d", dr->getAt(k));
            wrefresh(die); //dr->getAt(k) is the actual value of the roll
            delwin(die);
        }
        totalSum += dr->sum;
        totalRoll += '\n';
        statsWin = newwin(2, 32, lastY, (7 + (j * (numLen + 3))));
        acesMsg = "| " + std::to_string(dr->getNumAces()) + " ace(s)";
        mvwprintw(statsWin, 0, 0, "Sum: %d %s", dr->sum, (controller.isAcing() ?
                                                      acesMsg.c_str() : ""));
        mvwprintw(statsWin, 1, 0, "<-- %dd%d", dr->reps, dr->dieType);
        wrefresh(statsWin);
        delwin(statsWin);
        delete dr;
    }
    if (allRolls->size() > 1) {
        totalWin = newwin(1, 32, lastY + 3, 5);
        mvwprintw(totalWin, 0, 0, "Sum of all rolls: %d", totalSum);
        wrefresh(totalWin);
        delwin(totalWin);
    }
    allRolls->clear();
    delete allRolls;
    controller.logRolls(totalRoll);
}

/*
 * ==================== general menu printing ====================
 */

void printVMenu(int& highlight, int& input, WINDOW* win, const char **choices,
                int size, short offset) {
    for (int i = 0; i < size; i++) {
        if (i == highlight) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, i + offset, 1, choices[i]);
        wrefresh(win);
        wattroff(win, A_REVERSE);
    }
    input = wgetch(win);
    boundsCheckVMenu(size, highlight, input);
}

void printHMenu(int& highlight, int& input, WINDOW* win, const char **choices,
                int size, short offset) {
    int entryLen = 1;
    for (int i = 0; i < size; i++) {
        if (i == highlight) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, offset, entryLen, "%s", choices[i]);
        entryLen += strlen(choices[i]) + 2;
        wrefresh(win);
        wattroff(win, A_REVERSE);
    }
    input = wgetch(win);
    boundsCheckHMenu(size, highlight, input);
}

void boundsCheckVMenu(int numChoices, int &highlight,
                      const int input) {
    switch (input) {
        case KEY_UP:
            highlight--;
            highlight = (highlight < 0) ? numChoices - 1 : highlight;
            break;
        case KEY_DOWN:
            highlight++;
            highlight = (highlight == numChoices) ? 0 : highlight;
            break;
        default:
            break;
    }
}

void boundsCheckHMenu(int numChoices, int &highlight,
                      const int input) {
    switch (input) {
        case KEY_LEFT:
            highlight--;
            highlight = (highlight < 0) ? numChoices - 1 : highlight;
            break;
        case KEY_RIGHT:
            highlight++;
            highlight = (highlight == numChoices) ? 0 : highlight;
            break;
        default:
            break;
    }
}

/*
 * ==================== specific window setup ====================
 */

void setupInputWin(WINDOW *&inWin, bool redo, char *roll,
        DiceController &controller) {
    int xMax = getmaxx(stdscr);
    std::string rollValue;
    if (inWin == nullptr) {
        inWin = newwin(4, xMax - 12, 2, 5);
    }
    wclear(inWin);
    box(inWin, 0, 0);
    mvwprintw(inWin, 1, 1, "Enter your roll or the name of a saved roll:");
    if (!redo) {
        echo();
        curs_set(1);
        mvwgetnstr(inWin, 2, 1, roll, ROLL_VAL_MAX);
        rollValue = controller.getSavedRoll(roll);
        while (!controller.isValidRollVal(roll) && rollValue.empty()) {
            indicateError(inWin, 1, 1, 2, 1, "Invalid roll format");
            box(inWin, 0, 0);
            mvwprintw(inWin, 1, 1, "Enter your roll or the name of a saved roll:");
            mvwgetnstr(inWin, 2, 1, roll, ROLL_VAL_MAX);
            rollValue = controller.getSavedRoll(roll);
        }
        if (!rollValue.empty()) {
            strncpy(roll, rollValue.c_str(), ROLL_VAL_MAX);
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

void setUpWhatDo(WINDOW *&whatDo) {
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

void displayInputWin(WINDOW *inWin, char *roll) {
    box(inWin, 0, 0);
    mvwprintw(inWin, 1, 1, "Enter your roll:");
    mvwprintw(inWin, 2, 1, roll);
    curs_set(0);
    noecho();
}

/*
 * ==================== settings and its submenus ====================
 */

void handleSettings(DiceController &controller) {
    int highlight, input;
    highlight = input = 0;
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
        printVMenu(highlight, input, setWin, choices, numSettings, 1);
        if (input == 10) { //user pressed enter
            if (highlight == 0) { //change roll delay
                changeRollDelay(setWin, controller);
            } else if (highlight == 1) { //clear log
                clearRollLog(setWin, controller);
            } else if (highlight == 2) { //toggle aces
                toggleAces(setWin, controller);
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

void changeRollDelay(WINDOW *setWin, DiceController &controller) {
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
        indicateError(setWin, 1, 1, 1, 1, "Invalid input");
    }
}

void clearRollLog(WINDOW *setWin, DiceController &controller) {
    mvwprintw(setWin, 2, 1, "Really clear log? y/[n]");
    char real[3];
    echo();
    curs_set(1);
    mvwgetnstr(setWin, 2, 26, real, 3);
    if (strcmp(real, "y") == 0 || strcmp(real, "Y") == 0) {
        controller.clearLog();
    }
}

void toggleAces(WINDOW *setWin, DiceController &controller) {
    controller.toggleAces();
    wmove(setWin, 3, 1);
    wclrtoeol(setWin);
    box(setWin, 0, 0);
    mvwprintw(setWin, 3, 1, "Aces now %s", (controller.isAcing()) ? "on" : "off");
    box(setWin, 0, 0);
    wrefresh(setWin);
    usleep(1500000);
}

/*
 * ==================== saved rolls and its submenus ====================
 */

void handleSavedRolls(DiceController &controller) {
    int highlight, input;
    auto choices = controller.getKeys();
    choices->push_back("New roll...");
    choices->push_back("Exit saved rolls");
    highlight = input = 0;
    int numChoices = choices->size();
    WINDOW* rollsWin = newwin(numChoices + 2, ROLL_TOTAL_MAX, lastY + 4, 46);
    lastY += numChoices + 7;
    keypad(rollsWin, true);
    box(rollsWin, 0, 0);
    while (true) {
        printSavedRollMenu(rollsWin, *choices, highlight, input,
                           controller);
        if (input == ENTER) {
            if (highlight == choices->size() - 1) break;
            else if (highlight < controller.getNumRolls()) { //selected a saved roll
                WINDOW *optionsWin = newwin(1, ROLL_TOTAL_MAX, lastY - 1, 46);
                keypad(optionsWin, true);
                const char *subChoices[] = { "Rename", "Redefine", "Delete", "Cancel"};
                int subHL = 0, subInp = 0;
                while (true) {
                    printHMenu(subHL, subInp, optionsWin, subChoices,
                               4, 0);
                    if (subInp == ENTER) {
                        if (subHL == 2) { //delete
                            rollDelete(rollsWin, 0, choices, controller);
                            lastY--;
                        } else if (subHL == 1) {
                            rollRedefine(rollsWin, highlight, controller);
                        } else if (subHL == 0) {
                            rollRename(optionsWin, highlight, controller,
                                       choices);
                        }
                        break;
                    }
                }
                wclear(optionsWin);
                wrefresh(optionsWin);
                delwin(optionsWin);
            } else if (highlight == choices->size() - 2) {
                newRoll(rollsWin, highlight, controller, choices);
                lastY++;
            }
        }
    }
    wclear(rollsWin);
    wrefresh(rollsWin);
    delwin(rollsWin);
    choices->clear();
    delete choices;
}

void printSavedRollMenu(WINDOW *rollsWin, std::vector<std::string> &choices,
                        int &highlight, int &input,
                        DiceController &controller) {
    wclear(rollsWin);
    box(rollsWin, 0, 0);
    for (int i = 0; i < choices.size(); i++) {
        if (i == highlight) {
            wattron(rollsWin, A_REVERSE);
        }
        if (controller.savedRollExists(choices[i])) {
            wattron(rollsWin, A_BOLD);
            mvwprintw(rollsWin, i + 1, 1, "%s: ", choices[i].c_str());
            wattroff(rollsWin, A_BOLD);
            wprintw(rollsWin, controller.getSavedRoll(choices[i]).c_str());
        } else { //either "New roll..." or "Exit saved rolls"
            mvwprintw(rollsWin, i + 1, 1, choices[i].c_str());
        }
        wrefresh(rollsWin);
        wattroff(rollsWin, A_REVERSE);
    }
    input = wgetch(rollsWin);
    boundsCheckVMenu(choices.size(), highlight, input);
}

void rollRename(WINDOW *optWin, int &highlight, DiceController &controller,
                std::vector<std::string> *choices) {
    auto keys = controller.getKeys();
    std::string oldRollName = keys->at(highlight);
    std::string oldRollVal = controller.getSavedRoll(oldRollName);
    delete keys;
    wclear(optWin);
    echo();
    curs_set(true);
    char newName[ROLL_NAME_MAX];
    mvwprintw(optWin, 0, 1, "Rename \"%s\" to: ", oldRollName.c_str());
    mvwgetnstr(optWin, 0, 15 + oldRollName.length(), newName, ROLL_NAME_MAX);
    while (!controller.isValidRollName(newName)) {
        indicateError(optWin, 0, 1, 0, 1, "That name is taken or invalid");
        wclear(optWin);
        mvwprintw(optWin, 0, 1, "Rename \"%s\" to: ", oldRollName.c_str());
        mvwgetnstr(optWin, 0, 15 + oldRollName.length(), newName, ROLL_NAME_MAX);
    }
    controller.removeRoll(oldRollName);
    controller.addRoll(newName, oldRollVal);
    wclear(optWin);
    wrefresh(optWin);
    curs_set(false);
    choices->at(highlight) = newName;
    std::sort(choices->begin(), choices->end() - 2);
}

void rollRedefine(WINDOW *rollsWin, int highlight, DiceController &controller) {
    auto keys = controller.getKeys();
    std::string rollName = keys->at(highlight);
    delete keys;
    curs_set(true);
    echo();
    int y = highlight + 1;
    int x = rollName.length() + 3;
    wmove(rollsWin, y, x);
    wclrtoeol(rollsWin);
    wrefresh(rollsWin);
    char newRoll[ROLL_VAL_MAX];
    wgetnstr(rollsWin, newRoll, ROLL_VAL_MAX);
    while (!controller.isValidRollVal(newRoll)) {
        indicateError(rollsWin, y, x, y, x, "Invalid roll format");
        box(rollsWin, 0, 0);
        mvwgetnstr(rollsWin, highlight + 1, rollName.length() + 2, newRoll, ROLL_VAL_MAX);
    }
    controller.updateRoll(rollName, newRoll);
    curs_set(false);
    noecho();
}

void newRoll(WINDOW *rollsWin, int &highlight, DiceController &controller, std::vector<std::string> *choices) {
    int y = highlight + 1;
    int x = 1;
    wmove(rollsWin, y, x);
    wclrtoeol(rollsWin);
    box(rollsWin, 0, 0);
    echo();
    curs_set(true);
    char newRollName[ROLL_NAME_MAX];
    char newRollValue[ROLL_VAL_MAX];
    mvwgetnstr(rollsWin, y, x, newRollName, ROLL_NAME_MAX);
    while (!controller.isValidRollName(newRollName)) {
        indicateError(rollsWin, y, 1, y, 1, "Bad roll name");
        mvwgetnstr(rollsWin, y, x, newRollName, ROLL_NAME_MAX);
    }
    x += strlen(newRollName);
    mvwprintw(rollsWin, highlight + 1, x, ":");
    x += 2;
    mvwgetnstr(rollsWin, y, x, newRollValue, ROLL_VAL_MAX);
    while (!controller.isValidRollVal(newRollValue)) {
        indicateError(rollsWin, y, 1, y, 1, "Invalid roll format");
        box(rollsWin, 0, 0);
        mvwgetnstr(rollsWin, y, x, newRollValue, ROLL_VAL_MAX);
    }
    controller.addRoll(newRollName, newRollValue);
    curs_set(false);
    choices->insert(choices->begin(), newRollName);
    std::sort(choices->begin(), choices->end() - 2);
    wclear(rollsWin);
    wresize(rollsWin, choices->size() + 2, ROLL_TOTAL_MAX);
}

void rollDelete(WINDOW *rollsWin, int highlight, std::vector<std::string> *choices,
                DiceController &controller) {
    controller.removeRoll(choices->at(highlight));
    choices->erase(choices->begin() + highlight);
    wclear(rollsWin);
    wrefresh(rollsWin);
    wresize(rollsWin, choices->size() + 2, ROLL_TOTAL_MAX);
}

/*
 * ==================== miscellaneous ====================
 */

int getNumLength(int n) {
    if (n == 0) {
        return 0;
    } else {
        return 1 + getNumLength(n / 10);
    }
}

void indicateError(WINDOW *const &window, int problemY, int problemX, int originalY,
                   int originalX, const char *msg) {
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
