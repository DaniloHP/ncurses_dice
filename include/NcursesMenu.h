//
// Created by danilo on 5/24/20.
//

#ifndef NCURSES_NCURSESMENU_H
#define NCURSES_NCURSESMENU_H

#include <ncurses.h>
#include <vector>
#include <string>
#include "DiceController.h"

class NcursesMenu {
public:
    NcursesMenu(const char **choices, int size, int height, int width, int y, int x);
    NcursesMenu(const char **choices, int size);
    ~NcursesMenu();
    void printV(int& hl, int& input, short offset);
    void printH(int &hl, int &input, short offset);
    void add(std::string &entry);
    void add(std::string &entry, int index);
    void replace(std::string &entry, int index);
    void erase(int index);
    void move(int y, int x);
    void refresh();
    void resize(int height, int width);

private:
    WINDOW *win;
    std::vector<std::string> entries;
    void boundsCheck(int &hl, int input, bool vertical);

};

typedef NcursesMenu Menu;

#endif //NCURSES_NCURSESMENU_H
