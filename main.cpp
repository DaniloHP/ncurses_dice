
#include <ncurses.h>
#include <regex>
#include <vector>
#include <unistd.h>
#include <ctime>

using namespace std;

vector<int>& parseRoll(char* roll) {
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

int getNumLength(int n) {
    if (n == 0) {
        return 0;
    } else {
        return 1 + getNumLength(n / 10);
    }
}

void logRolls(string rolls) {
   time_t now = time(0);
   string date = ctime(&now);
   string command = "echo \"" + date + rolls +"\" >> rollHistory";
   system(command.c_str());
}

int main() {
    initscr();
    cbreak();

    char lastRoll[40];
    bool redo, done;
    done = redo = false;
    while (!done) {
        int xMax = getmaxx(stdscr);
        WINDOW *menuwin = newwin(4, xMax -12, 2, 5);
        box(menuwin, 0,0);
        refresh();
        mvwprintw(menuwin, 1, 1, "Enter your roll:");
        char roll[40];
        if (redo) {
            mvwprintw(menuwin, 2, 1, roll);
        } else {
            echo();
            curs_set(1);
            mvwgetstr(menuwin,2,1, roll);
        }
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
        string totalRoll = "";
        for (int i = 0; i < rollNums.size(); i += 2) {
            int dieType = rollNums.at(i + 1), numLen = getNumLength(dieType);
            int reps = rollNums.at(i);
            totalRoll += to_string(reps) + "d" + to_string(dieType) + ": ";
            lastY = 7 + (i * 2);
            int j;
            for (j = 0; j < reps; j++) {
                usleep(100000);
                WINDOW *die = newwin(3, numLen + 2, lastY, 5 + (j * (numLen + 3)));
                box(die, 0, 0);
                int roll = 1 + rand() % dieType;
                totalRoll = totalRoll + to_string(roll) + " ";
                mvwprintw(die, 1,1, "%d", roll);
                wrefresh(die);
            }
            totalRoll += "\n";
            mvprintw(lastY + 1,  (7 + (j * (numLen + 3))), "<-- %dd%d", reps,
                     dieType);
            refresh();
        }
        logRolls(totalRoll);

        WINDOW *whatDo = newwin(6, 40, lastY + 4, 5);
        box(whatDo, 0, 0);
        wattron(whatDo, A_BOLD);
        mvwprintw(whatDo, 1, 1, "What do?");
        wattroff(whatDo, A_BOLD);
        wrefresh(whatDo);
        string choices[] = {"Roll again", "Reroll this roll", "Exit"};
        int choice, highlight = redo ? 1 : 0;
        while (true) {
            for (int i = 0; i < 3; i++) {
                if (i == highlight) {
                    wattron(whatDo, A_REVERSE);
                }
                mvwprintw(whatDo, i + 2, 1, choices[i].c_str());
                wrefresh(whatDo);
                wattroff(whatDo, A_REVERSE);
            }
            choice = wgetch(menuwin); //not quite right should be from whatDo
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
