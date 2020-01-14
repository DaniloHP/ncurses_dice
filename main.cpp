
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
    int choice, highlight = 0;
    while (true) {
        for (int i = 0; i < rollNums.size() + 1; i++) {
            if (i == highlight) {
                wattron(menuwin, A_REVERSE);
            }
            wrefresh(rollWin);
            mvwaddstr(rollWin, i+1, 1, to_string(rollNums[i]).c_str());
            wattroff(menuwin, A_REVERSE);
        }
        
        choice = wgetch(menuwin);
        switch (choice) {
        case KEY_UP:
            highlight--;
            highlight = (highlight == -1) ? 2 : highlight;
            break;
        case KEY_DOWN:
            highlight++;
            highlight = (highlight == 4) ? 0 : highlight;
            break;    
        default:
            break;
        }

        if (choice == 10) {
            break;
        }
        wrefresh(rollWin);
    }
    
    //getch();
    endwin();
    return 0;
}
