#include "../../include/NcursesMenu.h"

NcursesMenu::NcursesMenu(const char **choices, int size) {
    for (int i = 0; i < size; i++) {
        this->entries.emplace_back(choices[i]);
    }
    win = newwin(0, 0, 0, 0);
}

NcursesMenu::NcursesMenu(const char **choices,
                         int size, int height, int width, int y, int x) {
    for (int i = 0; i < size; i++) {
        this->entries.emplace_back(choices[i]);
    }
    win = newwin(height, width, y, x);
}

Menu::~NcursesMenu() {
    delwin(win);
    entries.clear();
}

void Menu::move(int y, int x) {
    wclear(win);
    mvwin(win, y, x);
    wrefresh(win);
}

void Menu::refresh() {
    wrefresh(win);
    entries.clear();
}

void Menu::resize(int height, int width) {
    wresize(win, height, width);
}

void Menu::printV(int& hl, int& input, short offset) {
    for (int i = 0; i < entries.size(); i++) {
        if (i == hl) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, i + offset, 1, entries[i].c_str());
        wrefresh(win);
        wattroff(win, A_REVERSE);
    }
    input = wgetch(win);
    boundsCheck(hl, input, true);
}

void Menu::printH(int& hl, int& input, short offset) {
    int entryLen = 1;
    for (int i = 0; i < entries.size(); i++) {
        if (i == hl) {
            wattron(win, A_REVERSE);
        }
        mvwprintw(win, offset, entryLen, "%s", entries[i].c_str());
        entryLen += entries[i].length() + 2;
        wrefresh(win);
        wattroff(win, A_REVERSE);
    }
    input = wgetch(win);
    boundsCheck(hl, input, false);
}

void Menu::boundsCheck(int &hl, int input, bool vertical) {
    int next = vertical ? KEY_UP : KEY_RIGHT;
    int prev = vertical ? KEY_DOWN : KEY_LEFT;
    if (input == next) {
        hl--;
        hl = (hl < 0) ? entries.size() - 1 : hl;
    } else if (input == prev) {
        hl++;
        hl = (hl == entries.size()) ? 0 : hl;
    }
}

void Menu::add(std::string &entry) {
    entries.emplace_back(entry);
}

void Menu::add(std::string &entry, int index) {
    entries.insert(entries.begin() + index, entry);
}

void Menu::replace(std::string &entry, int index) {
    entries.at(index) = entry;
}

void Menu::erase(int index) {
    entries.erase(entries.begin() + index);
}

