
#include <ncurses.h>
#include <regex>
#include <vector>

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

int getNumLength(int n) {
    if (n == 0) {
        return 0;
    } else {
        return 1 + getNumLength(n / 10);
    }
}

int main() {
    initscr();
    cbreak();

    int xMax = getmaxx(stdscr);
    WINDOW *menuwin = newwin(4, xMax -12, 2, 5);

    box(menuwin, 0,0);
    refresh();
    mvwaddstr(menuwin, 1, 1, "Enter your roll:");
    char roll[40];
    mvwgetstr(menuwin,2,1, roll);
    wrefresh(menuwin);

    vector<int> rollNums = parseRoll(roll);

    refresh();
    keypad(menuwin, true);
    curs_set(0);
    noecho();

    WINDOW *rollWin = newwin(rollNums.size() + 2, 10, 7, 5);
    box(rollWin, 0, 0);
    srand (time(NULL));
    int lastY = 11; //minimum lastY
    for (int i = 0; i < rollNums.size(); i += 2) {
        unsigned int dieType = rollNums.at(i + 1), numLen = getNumLength(dieType);
        lastY = 7 + (i * 2);
        for (int j = 0; j < rollNums.at(i); j++) {
            WINDOW *die = newwin(3, numLen + 2, lastY, 5 + (j * (numLen + 3)));
            box(die, 0, 0);
            int roll = 1 + rand() % dieType;
            mvwprintw(die, 1,1, "%d", roll);
            wrefresh(die);
        }
    }

    WINDOW *whatDo = newwin(6, 40, lastY + 4, 5);
    box(whatDo, 0, 0);
    mvwprintw(whatDo, 1, 1, "What do?");
    wrefresh(whatDo);
    
    getch();
    endwin();
    return 0;
}
