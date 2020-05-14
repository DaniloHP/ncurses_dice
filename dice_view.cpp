#include <ncurses.h>
#include <regex>
#include <vector>
#include <unistd.h>
#include <iostream>
#include "dice_controller.h"
#define ENTER 10
#define DEFAULT_SLEEP 80000

using namespace std;
long delay_micro_sec = DEFAULT_SLEEP;

void print_menu(int &highlight, int &choice, WINDOW *win, const char **choices, int size, short offset);
void do_rolls(vector<int> *roll_nums, int &last_y, dice_controller &dc);
void setup_input_win(WINDOW *&in_win, bool redo, char *roll);
void set_up_what_do_win(int last_y, WINDOW *&what_do);
void handle_settings(int lastY, dice_controller &dc);
void display_input_win(WINDOW *in_win, char *roll);
int get_num_length(int);
void handle_change_roll_delay(WINDOW *set_win);
void handle_clear_log(WINDOW *set_win, dice_controller &dc);
void handle_toggle_aces(WINDOW *set_win);

bool aces = true;

int main() {
    WINDOW *what_do = nullptr, *input_win = nullptr, *main_screen = initscr();
    dice_controller dc;
    char roll[40];
    int last_y, choice, highlight;
    vector<int> roll_nums;
    bool redo, done, sett;
    done = redo = false;
    cbreak();
    while (!done) {
        setup_input_win(input_win, redo, roll);
        last_y = 11;
        do_rolls(dc.parse_roll(roll), last_y, dc);
        set_up_what_do_win(last_y, what_do); //good i think
        highlight = redo ? 1 : 0;
        while (true) {
            redo = sett = false; //git
            const char *choices[] = {
                    (const char*)"Roll again", (const char*)"Reroll this roll", 
                    (const char*)"Settings", (const char*)"Exit"};
            print_menu(highlight, choice, what_do, choices, 4, 2);
            if (choice == ENTER) { //pressed enter
                if (highlight == 3) { //exit
                    done = true;
                } else if (highlight == 2) { //settings
                    handle_settings(last_y, dc);
                    sett = true;
                } else if (highlight == 1) { //redo the roll
                    redo = true;
                }
                if (!sett) {
                    clear();
                    refresh();
                    display_input_win(input_win, roll);
                    wrefresh(input_win);
                    break;
                }
            }
        }
    }
    delwin(input_win);
    delwin(what_do);
    delwin(main_screen);
    endwin();
    vector<int>().swap(roll_nums);
    return 0;
}

void setup_input_win(WINDOW *&in_win, bool redo, char *roll) { 
    int x_max = getmaxx(stdscr);
    if (in_win == nullptr) {
        in_win = newwin(4, x_max - 12, 2, 5);
    }
    wclear(in_win);
    box(in_win, 0, 0);
    mvwprintw(in_win, 1, 1, "Enter your roll:");
    if (redo) {
        mvwprintw(in_win, 2, 1, roll);
    } else {
        echo();
        curs_set(1);
        mvwgetnstr(in_win, 2, 1, roll, 40);
    }
    curs_set(0);
    noecho();
}

void display_input_win(WINDOW *in_win, char *roll) { 
    box(in_win, 0, 0);
    mvwprintw(in_win, 1, 1, "Enter your roll:");
    mvwprintw(in_win, 2, 1, roll);
    curs_set(0);
    noecho();
} //wrefresh does not work in here, maybe because double pointer or something?

void set_up_what_do_win(int last_y, WINDOW *&what_do) { 
    if (what_do == nullptr) {
        what_do = newwin(7, 40, last_y + 4, 5);
    } else {
        mvwin(what_do, last_y + 4, 5);
    }
    keypad(what_do, true);
    box(what_do, 0, 0);
    wattron(what_do, A_BOLD);
    mvwprintw(what_do, 1, 1, "What do?");
    wattroff(what_do, A_BOLD);
    wrefresh(what_do);
}

void print_menu(int& highlight, int& choice, WINDOW* win, const char **choices, int size,
                short offset) { 
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

void handle_settings(int lastY, dice_controller &dc) { 
    int highlight, choice;
    highlight = choice = 0;
    const char *choices[] = {
            (const char *)"Set roll delay", (const char *)"Clear roll log", 
            (const char *)"Toggle aces", (const char *)"Exit settings"};
    int num_settings = 4;
    WINDOW* set_win = newwin(num_settings + 2, 40, lastY + 4, 46);
    keypad(set_win, true);
    box(set_win, 0, 0);
    while (true) {
        wclear(set_win);
        box(set_win, 0, 0);
        print_menu(highlight, choice, set_win, choices, num_settings, 1);
        if (choice == 10) { //use pressed enter
            if (highlight == 0) { //change roll delay
                handle_change_roll_delay(set_win);
            } else if (highlight == 1) { //clear log
                handle_clear_log(set_win, dc);
            } else if (highlight == 2) { //toggle aces
                handle_toggle_aces(set_win);
            } else if (highlight == num_settings - 1) { //exit settings
                break;
            }
            curs_set(0);
            noecho();
        } 
    }
    wclear(set_win);
    wrefresh(set_win);
    delwin(set_win);
}

void handle_change_roll_delay(WINDOW *set_win) {
    echo();
    curs_set(1);
    char new_val[10];
    wmove(set_win, 1, 1);
    wclrtoeol(set_win);
    box(set_win, 0, 0);
    mvwprintw(set_win, 1, 1, "Value (milliseconds):");
    box(set_win, 0, 0);
    mvwgetnstr(set_win, 1, 23, new_val, 10);
    try {
        long milli_sec = stol(new_val);
        if (milli_sec >= 0) {
            delay_micro_sec = milli_sec * 1000;
        } else {
            throw invalid_argument("No values less than 0.");
        }
    } catch (invalid_argument &e) {
        wmove(set_win, 1, 1);
        curs_set(0);
        wclrtoeol(set_win);
        wattron(set_win, A_BOLD);
        box(set_win, 0, 0);
        mvwprintw(set_win, 1, 1, "invalid input");
        wattroff(set_win, A_BOLD);
        wrefresh(set_win);
        usleep(1500000);
    }
}

void handle_clear_log(WINDOW *set_win, dice_controller &dc) {
    mvwprintw(set_win, 2, 1, "Really clear log? y/[n]");
    char real[3];
    echo();
    curs_set(1);
    mvwgetnstr(set_win, 2, 26, real, 3);
    if (strcmp(real, "y") == 0 || strcmp(real, "Y") == 0) {
        dc.clear_log();
    }
}

void handle_toggle_aces(WINDOW *set_win) {
    aces = !aces;
    wmove(set_win, 3, 1);
    wclrtoeol(set_win);
    box(set_win, 0, 0);
    mvwprintw(set_win, 3, 1, "Aces now %s", (aces) ? "on" : "off");
    box(set_win, 0, 0);
    wrefresh(set_win);
    usleep(1500000);
}

int get_num_length(int n) {
    if (n == 0) {
        return 0;
    } else {
        return 1 + get_num_length(n / 10);
    }
}

void do_rolls(vector<int> *roll_nums, int &last_y, dice_controller &dc) { 
    int sum, total_sum, orig_reps, die_type, reps, num_len, roll_val, i, j;
    WINDOW *die, *stats_win, *total_win;
    string total_roll;
    total_sum = 0;
    for (i = 0; i < roll_nums->size(); i += 2) {
        sum = 0;
        die_type = roll_nums->at(i + 1);
        num_len = get_num_length(die_type);
        reps = orig_reps = roll_nums->at(i);
        total_roll += to_string(reps) + "d" + to_string(die_type) + ": ";
        last_y = 7 + (i * 2);
        for (j = 0; j < reps; j++) {
            usleep(delay_micro_sec);
            die = newwin(3, num_len + 2, last_y, 5 + (j * (num_len + 3)));
            box(die, 0, 0);
            roll_val = dc.get_roll(die_type);
            sum += roll_val;
            total_roll += to_string(roll_val) + " ";
            mvwprintw(die, 1, 1, "%d", roll_val);
            wrefresh(die);
            delwin(die);
            if (aces && roll_val == die_type && die_type != 1) reps++;
        }
        total_sum += sum;
        total_roll += "\n";
        stats_win = newwin(2, 32, last_y, (7 + (j * (num_len + 3))));
        mvwprintw(stats_win, 0, 0, "Sum: %d | %s ace(s)", sum,
                  aces ? to_string(reps - orig_reps).c_str() : "No");
        mvwprintw(stats_win, 1, 0, "<-- %dd%d", reps,
                  die_type);
        wrefresh(stats_win);
        delwin(stats_win); //unfortunately these seem to have to be in windows
    }
    delete roll_nums;
    total_win = newwin(1, 32, last_y + 3, 5);
    mvwprintw(total_win, 0, 0, "Sum of all rolls: %d", total_sum);
    wrefresh(total_win);
    delwin(total_win);
    dc.log_rolls(total_roll);
    total_roll.erase(0);
}
