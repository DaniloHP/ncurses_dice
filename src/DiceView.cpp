#include <ncurses.h>
#include <cstring>
#include <algorithm>
#include <thread>
#include "DiceController.h"

#define ROLL_PROMPT "Enter your roll or the name of a saved roll:"
#define ENTER 10
// ^ ncurses.h's KEY_ENTER is not 10 for some reason

//print all the given rolls
void printRolls(const std::vector<DiceRoll> &allRolls, const DiceController &controller);

//general menu printing
void printVMenu(int &highlight, int &input, WINDOW *win, const char **choices,
                int size, short paddingTop);
void printHMenu(int &highlight, int &input, WINDOW *win, const char **choices, int size);
int getNewHighlight(int numChoices, int highlight, int input, bool vertical);

//specific window setup
void setupInputWin(WINDOW *&inWin, bool redo, char *roll,
                   DiceController &controller);
void setUpWhatDo(WINDOW *&whatDo);
void refillInputWin(WINDOW *inWin, char *roll);

//settings and its submenus
void handleSettings(DiceController &controller);
void changeRollDelay(WINDOW *setWin, DiceController &controller);
void clearRollLog(WINDOW *setWin, const DiceController &controller);
void toggleAces(WINDOW *setWin, DiceController &controller);

//saved rolls and its submenus
void handleSavedRolls(DiceController &controller);
void printSavedRollMenu(WINDOW *rollsWin, const std::vector<std::string> &choices,
                        int &highlight, int &input, DiceController &controller);
void rollRename(WINDOW *optWin, int highlight, std::vector<std::string> &choices, DiceController &controller);
void rollRedefine(WINDOW *rollsWin, int highlight, DiceController &controller);
void newRoll(WINDOW *rollsWin, int highlight, std::vector<std::string> &choices, DiceController &controller);
void rollDelete(WINDOW *rollsWin, int highlight, std::vector<std::string> &choices,
                DiceController &controller);

//miscellaneous
int  getNumLength(int n);
void indicateError(WINDOW *window, int problemY, int problemX, int originalY,
              int originalX, const char *msg);

static int lastY = 11;

int main() {
    WINDOW *whatDo = nullptr, *inputWin = nullptr, *mainScreen = initscr();
    DiceController controller;
    char roll[ROLL_VAL_MAX];
    int input, highlight;
    std::vector<DiceRoll> allRolls;
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
        highlight = redo ? 1 : 0; //if last roll was redo, start with reroll option HL'd
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
                    refillInputWin(inputWin, roll);
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
    return 0;
}

/**
 * Given a vector of DiceRoll objects, this function prints them all to the 
 * screen in little boxed ncurses windows.
 * 
 * @param allRolls The vector containing DiceRoll objects.
 * @param controller The DiceController object used to query the model.
 */
void printRolls(const std::vector<DiceRoll> &allRolls, const DiceController &controller) {
    int j, k, numLen, xMax = getmaxx(stdscr), i = 0, wraps = 0;
    unsigned long long int totalSum = 0;
    WINDOW *die, *statsWin, *totalWin;
    for (const DiceRoll &dr : allRolls) {
        numLen = getNumLength(dr.dieType);
        lastY = 7 + (3 * (wraps + i++));
        //k is the real iterator that checks for reps, j is for visual purposes
        for (j = 0, k = 0; k < dr.reps; j++, k++) {
            std::this_thread::sleep_for(std::chrono::microseconds(controller.getDelay()));
            if (5 + (j * (numLen + 3)) > xMax - 12) { //wrap rolls onto new line
 //left padding ^    ^^^^^^^^^^^^^^^^ width of all the rolls in the row
                j = 0, lastY += 3, wraps++;
            }
            die = newwin(3, numLen + 2, lastY, 5 + (j * (numLen + 3)));
            box(die, 0, 0);
            mvwprintw(die, 1, 1, "%d", dr.getAt(k));
            wrefresh(die); //dr->getAt(k) is the actual value of the roll
            delwin(die);
        }
        totalSum += dr.sum;
        statsWin = newwin(3, 32, lastY, (7 + (j * (numLen + 3))));
        mvwprintw(statsWin, 0, 0, "Sum: %llu", dr.sum);
        mvwprintw(statsWin, 1, 0, "<-- %dd%d", dr.origReps, dr.dieType);
        if (controller.isAcing()) {
            mvwprintw(statsWin, 2, 0, "Aces: %d", dr.getNumAces());
        }
        wrefresh(statsWin);
        delwin(statsWin);
    }
    if (allRolls.size() > 1) {
        totalWin = newwin(1, 32, lastY + 3, 5);
        mvwprintw(totalWin, 0, 0, "Sum of all rolls: %llu", totalSum);
        wrefresh(totalWin);
        delwin(totalWin);
    }
}

/*
 * ==================== general menu printing ====================
 */

/**
 * Prints a menu vertically with the entries contained in the given char* array.
 * The user can navigate the menu with the arrow keys.
 * @param highlight Int reference indicating the index of the option currently 
 * highlighted.
 * @param input Int reference holding user input (arrow up, down, etc).
 * @param win The ncurses windows in which to print the menu.
 * @param choices The menu entries, each printed on a new line.
 * @param size Size of choices.
 * @param paddingTop Number of lines from the top of the window to not print on
 * in case something is already there
 */
void printVMenu(int& highlight, int& input, WINDOW* win, const char **choices,
                const int size, const short paddingTop) {
    for (int i = 0; i < size; i++) {
        if (i == highlight) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, i + paddingTop, 1, choices[i]);
        wrefresh(win);
        wattroff(win, A_REVERSE);
    }
    input = wgetch(win);
    highlight = getNewHighlight(size, highlight, input, true);
}

/**
 * Prints a menu horizontally with the entries contained in the given char* array.
 * The user can navigate the menu with the arrow keys.
 * @param highlight Int reference indicating the index of the option currently 
 * highlighted.
 * @param input Int reference holding user input (arrow right, left, etc).
 * @param win The ncurses windows in which to print the menu.
 * @param choices The menu entries, each printed next to each other.
 * @param size Size of choices.
 */
void printHMenu(int &highlight, int &input, WINDOW *win, const char **choices,
        const int size) {
    int entryLen = 1;
    for (int i = 0; i < size; i++) {
        if (i == highlight) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, 0, entryLen, "%s", choices[i]);
        entryLen += strlen(choices[i]) + 2;
        wrefresh(win);
        wattroff(win, A_REVERSE);
    }
    input = wgetch(win);
    highlight = getNewHighlight(size, highlight, input, false);
}

/**
 * Given user input, checks to see how the highlighted index should be adjusted
 * and prevents highlight from going out of bounds.
 * 
 * @param numChoices The number of entries in the menu.
 * @param highlight The index of the menu entry currently highlighted
 * @param input The user's input.
 * @param vertical Whether this function should check bounds for a vertical or 
 * a horizontal menu.
 * 
 * @return the new index of the highlight after the user's input
 */
int getNewHighlight(const int numChoices, int highlight, const int input, 
        const bool vertical) {
    int next = vertical ? KEY_DOWN : KEY_RIGHT;
    int prev = vertical ? KEY_UP : KEY_LEFT;
    if (input == prev) {
        highlight--;
        highlight = (highlight < 0) ? numChoices - 1 : highlight;
    } else if (input == next) {
        highlight++;
        highlight = (highlight == numChoices) ? 0 : highlight;
    }
    return highlight;
}

/*
 * ==================== specific window setup ====================
 */

/**
 * This function sets up and collects input from the inputWin, which is the
 * in which the user enters their roll.
 * 
 * @param inWin The inputWin ncurses window.
 * @param redo Boolean indicating if the user chose to reroll the last roll or
 * chose a saved roll.
 * @param roll The roll the user LAST entered.
 * @param controller The DiceController object used for validating input and
 * checking for saved rolls.
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
    mvwprintw(inWin, 1, 1, ROLL_PROMPT);
    if (!redo) {
        echo();
        curs_set(1);
        mvwgetnstr(inWin, 2, 1, roll, ROLL_VAL_MAX);
        rollValue = controller.getSavedRoll(roll);
        while (!controller.isValidRollVal(roll) && rollValue.empty()) {
            indicateError(inWin, 1, 1, 2, 1, "Invalid roll format");
            mvwprintw(inWin, 1, 1, ROLL_PROMPT);
            mvwgetnstr(inWin, 2, 1, roll, ROLL_VAL_MAX);
            rollValue = controller.getSavedRoll(roll);
        }
        if (!rollValue.empty()) {
            redo = true;
            int rollNameLen = strnlen(roll, ROLL_NAME_MAX);
            strncpy(roll, rollValue.c_str(), ROLL_VAL_MAX);
            mvwprintw(inWin, 2, rollNameLen + 2, "(%s)", roll);
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

/**
 * Sets up the whatDo window but does not print the menu itself.
 * 
 * @param whatDo The whatDo window
 */
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

/**
 * This function only makes sure all of the content of the input window is there,
 * so that it can be wrefreshed back in main.
 * For some reason, without this, the input window disappears when you choose 
 * to reroll.
 * 
 * @param inWin The input window.
 * @param roll The last roll inputted by the user.
 */
void refillInputWin(WINDOW *inWin, char *roll) {
    box(inWin, 0, 0);
    mvwprintw(inWin, 1, 1, ROLL_PROMPT);
    mvwprintw(inWin, 2, 1, roll);
    curs_set(0);
    noecho();
}

/*
 * ==================== settings and its submenus ====================
 */

/**
 * Like the main function for the settings menu, this function handles every
 * part of the settings menu and is not left until the settings menu is exited.
 * Each option has its own handler of sorts, which is called from here.
 * 
 * @param controller The DiceController object used here to adjust settings and
 * interact with the rolls log.
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

/**
 * Goes through the process of adjusting the delay between eac die being displayed,
 * including input sanitization.
 * 
 * @param setWin The settings window
 * @param controller The controller for adjusting the delay in the model as well
 */
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

/**
 * Allows the user to delete the rolls log.
 * 
 * @param setWin The settings window.
 * @param controller The controller, which can actually truncate the file.
 */
void clearRollLog(WINDOW *setWin, const DiceController &controller) {
    mvwprintw(setWin, 2, 1, "Really clear log? y/[n]");
    char real[3];
    echo();
    curs_set(1);
    mvwgetnstr(setWin, 2, 26, real, 3);
    if (strcmp(real, "y") == 0 || strcmp(real, "Y") == 0) {
        controller.clearLog();
    }
}

/**
 * Allows the user to toggle aces on or off.
 *
 * @param setWin The settings window.
 * @param controller The controller which knows what the value is at now and can
 * toggle it.
 */
void toggleAces(WINDOW *setWin, DiceController &controller) {
    controller.toggleAces();
    wmove(setWin, 3, 1);
    wclrtoeol(setWin);
    box(setWin, 0, 0);
    mvwprintw(setWin, 3, 1, "Aces now %s", (controller.isAcing()) ? "on" : "off");
    box(setWin, 0, 0);
    wrefresh(setWin);
    std::this_thread::sleep_for(std::chrono::microseconds(1500000));
}

/*
 * ==================== saved rolls and its submenus ====================
 */

/**
 * This is like handleSettings but for the saved rolls menu, which is a list of
 * saved rolls and contains a submenu for operating on the rolls. 
 * This menu is too specialized to be generalized into fitting with the general
 * print menu function.
 * 
 * @param controller The controller object which will be used for querying and 
 * modifying saved rolls in the model.
 */
void handleSavedRolls(DiceController &controller) {
    int oLastY = lastY;
    int highlight, input;
    auto choices = controller.getKeys();
    choices.emplace_back("New roll...");
    choices.emplace_back("Exit saved rolls");
    highlight = input = 0;
    int numChoices = choices.size();
    WINDOW* rollsWin = newwin(numChoices + 2, ROLL_TOTAL_MAX, lastY + 4, 46);
    lastY += numChoices + 7;
    keypad(rollsWin, true);
    box(rollsWin, 0, 0);
    while (true) {
        printSavedRollMenu(rollsWin, choices, highlight, input,
                           controller);
        if (input == ENTER) {
            if (highlight == choices.size() - 1) break;
            else if (highlight < controller.getNumRolls()) { //selected a saved roll
                WINDOW *optionsWin = newwin(1, ROLL_TOTAL_MAX, lastY - 1, 46);
                keypad(optionsWin, true);
                const char *subChoices[] = { "<Rename>", "<Redefine>", 
                                             "<Delete>", "<Cancel>"};
                int subHL = 0, subInp = 0;
                while (true) {
                    printHMenu(subHL, subInp, optionsWin, subChoices,
                               4);
                    if (subInp == ENTER) {
                        if (subHL == 2) { //delete
                            rollDelete(rollsWin, 0, choices, controller);
                            lastY--;
                        } else if (subHL == 1) {
                            rollRedefine(rollsWin, highlight, controller);
                        } else if (subHL == 0) {
                            rollRename(optionsWin, highlight,
                                       choices, controller);
                        }
                        break;
                    }
                }
                wclear(optionsWin);
                wrefresh(optionsWin);
                delwin(optionsWin);
            } else if (highlight == choices.size() - 2) {
                newRoll(rollsWin, highlight, choices, controller);
                lastY++;
            }
        } else if (input == KEY_DC && highlight < controller.getNumRolls()) {
            rollDelete(rollsWin, highlight, choices, controller);  
        } //delete rolls by pressing the delete key
    }
    wclear(rollsWin);
    wrefresh(rollsWin);
    delwin(rollsWin);
    choices.clear();
    lastY = oLastY;
}

/**
 * Prints the saved rolls with special formatting.
 * 
 * @param rollsWin The saved rolls window.
 * @param choices The vector of choices in the menu.
 * @param highlight Int reference indicating the index of the option currently 
 * highlighted.
 * @param input Int reference holding user input (arrow right, left, etc).
 * @param controller The controller, used here to get saved roll values
 */
void printSavedRollMenu(WINDOW *rollsWin, const std::vector<std::string> &choices,
                        int &highlight, int &input,
                        DiceController &controller) {
    wclear(rollsWin);
    box(rollsWin, 0, 0);
    for (int i = 0; i < choices.size(); i++) {
        if (i == highlight) {
            wattron(rollsWin, A_REVERSE);
        } if (controller.savedRollExists(choices[i])) {
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
    highlight = getNewHighlight(choices.size(), highlight, input, true);
}

/**
 * This function handles the renaming of saved rolls.
 * 
 * @param optWin A secondary window below the saved rolls window in which the 
 * roll is renamed.
 * @param highlight The index which was highlighted when the user entered this
 * menu
 * @param choices The pointer to the vector of all the menu items, which 
 * contains roll keys.
 * @param controller The controller which will rename the roll in the model and
 * validates the new name.
 */
void rollRename(WINDOW *optWin, const int highlight, 
        std::vector<std::string> &choices, DiceController &controller) {
    auto keys = controller.getKeys();
    std::string oldRollName = keys.at(highlight);
    std::string oldRollVal = controller.getSavedRoll(oldRollName);
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
    choices.at(highlight) = newName;
    std::sort(choices.begin(), choices.end() - 2);
}

/**
 * This function handles the redefinition of saved rolls' values.
 * 
 * @param rollsWin The saved rolls window.
 * @param highlight The index which was highlighted when the user entered this
 * function
 * @param controller The controller which will redefine the roll in the model 
 * and validates the new value.
 */
void rollRedefine(WINDOW *rollsWin, const int highlight, DiceController &controller) {
    auto keys = controller.getKeys();
    std::string rollName = keys.at(highlight);
    curs_set(true);
    echo();
    int y = highlight + 1;
    int x = rollName.length() + 3;
    wmove(rollsWin, y, 1);
    wclrtoeol(rollsWin);
    box(rollsWin, 0, 0);
    mvwprintw(rollsWin, y, 1, "%s:", rollName.c_str());
    wrefresh(rollsWin);
    char newRoll[ROLL_VAL_MAX];
    mvwgetnstr(rollsWin, y, x, newRoll, ROLL_VAL_MAX);
    while (!controller.isValidRollVal(newRoll)) {
        indicateError(rollsWin, y, x, y, x, "Invalid roll format");
        mvwgetnstr(rollsWin, y, x, newRoll, ROLL_VAL_MAX);
    }
    controller.updateRoll(rollName, newRoll);
    curs_set(false);
    noecho();
}

/**
 * This function handles adding a new saved roll.
 * 
 * @param rollsWin the saved rolls window.
 * @param highlight The index which was highlighted when the user entered this
 * function
 * @param choices The pointer to the vector of all the menu items, which 
 * contains roll keys.
 * @param controller The controller which will add the new the roll in the model
 * and validates the new name and value.
 */
void newRoll(WINDOW *rollsWin, const int highlight, 
        std::vector<std::string> &choices, DiceController &controller) {
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
    int rollNameLen = strnlen(newRollName, ROLL_NAME_MAX);
    x += rollNameLen;
    mvwprintw(rollsWin, highlight + 1, x, ":");
    mvwgetnstr(rollsWin, y, x + 2, newRollValue, ROLL_VAL_MAX);
    while (!controller.isValidRollVal(newRollValue)) {
        indicateError(rollsWin, y, 1, y, 1, "Invalid roll format");
        mvwprintw(rollsWin, y, x - rollNameLen, "%s:", newRollName); //reprint name since ^ clears the line
        box(rollsWin, 0, 0);
        mvwgetnstr(rollsWin, y, x + 2, newRollValue, ROLL_VAL_MAX);
    }
    controller.addRoll(newRollName, newRollValue);
    curs_set(false);
    choices.insert(std::lower_bound(
        choices.begin(), choices.end() - 2, newRollName), newRollName);
    wclear(rollsWin);
    wresize(rollsWin, choices.size() + 2, ROLL_TOTAL_MAX);
}

/**
 * This function handles the deletion of saved rolls.
 *
 * @param rollsWin the saved rolls window.
 * @param highlight The index which was highlighted when the user entered this
 * menu
 * @param choices The pointer to the vector of all the menu items, which 
 * contains roll keys.
 * @param controller The controller which will delete the roll from the model.
 */
void rollDelete(WINDOW *rollsWin, const int highlight, 
        std::vector<std::string> &choices, DiceController &controller) {
    controller.removeRoll(choices.at(highlight));
    choices.erase(choices.begin() + highlight);
    wclear(rollsWin);
    wrefresh(rollsWin);
    wresize(rollsWin, choices.size() + 2, ROLL_TOTAL_MAX);
}

/*
 * ==================== miscellaneous ====================
 */

/**
 * This function gets the number of digits in an int n.
 * 
 * @param n The number whose length to check.
 * 
 * @return The length of n.
 */
int getNumLength(const int n) {
    if (n == 0) {
        return 0;
    } else {
        return 1 + getNumLength(n / 10);
    }
}

/**
 * This function indicates an error on the given window.
 * It sleeps the program temporarily to make it more obvious on terminal 
 * emulators where the attribute A_BOLD does not change the printed 
 * text's color. All x and y are relative to the given window.
 * 
 * @param window The window on which to indicate an error.
 * @param problemY The y where some error occurred. 
 * @param problemX The x where some error occurred. 
 * @param originalY The y where the cursor should be returned afterward. 
 * @param originalX The x where the cursor should be returned afterward. 
 * @param msg The message which will be printed at problemY, problemX.
 */
void indicateError(WINDOW *window, const int problemY, const int problemX, 
        const int originalY, const int originalX, const char *msg) {
    wmove(window, problemY, problemX);
    curs_set(0);
    wclrtoeol(window);
    wattron(window, A_BOLD);
    box(window, 0, 0);
    mvwprintw(window, problemY, problemX, msg);
    wattroff(window, A_BOLD);
    wrefresh(window);
    std::this_thread::sleep_for(std::chrono::microseconds(1500000));
    wmove(window, problemY, problemX);
    wclrtoeol(window);
    wmove(window, originalY, originalX);
    wclrtoeol(window);
    box(window, 0, 0);
    wrefresh(window);
    curs_set(1);
}
