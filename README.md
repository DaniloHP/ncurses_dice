#Welcome to Ncurses Dice
This is a very simple dice roller with the functionality for saving rolls you use 
frequently and the use of "aces," a rule where dice rolled at their highest value
are rolled again. This is a rule in tabletop RPG system Savage Worlds, and probably
others. It also saves all your rolls to a text file called RollHistory.txt which 
will appear in the project root directory the first time you roll. If you want it
to be somewhere else, you can adjust that in config.ini under the key `rollLogSavePath`.

This program makes heavy use of ncurses, a library for POSIX systems that 
facilitates the creation of terminal-based user interfaces. Compatibility, then,
is tricky for Windows, but I've run it with no problem on WSL Debian. Additionally, 
I've tested it on Arch Linux and OpenSUSE Tumbleweed. 

##Build
To get started, make sure you have `cmake` and `make` installed on your system. 
In the project root directory, run `cmake CMakeLists.txt` and then `make`.
Hopefully this will build the project, at which point you can run the executable
`bin/dice` in your favorite terminal emulator. It's possible that your distro
doesn't come with ncurses installed, so make sure you get that installed as well. 
The required package may be called something like `libncurses` or `ncurses-devel`.

##Use
To roll, simply enter one or more dice rolls in the following format:
>`2d10 d20 3d6`

This will roll two d10s, one d20, and three d6s. Dice without a quantifier like 
that d10 will be rolled once. Technically the limit to both how many and what 
kind of dice you can roll is the signed int max, but you will quickly run off 
the page 
if rolling more than 300-500 dice at a time. 

If there's a roll that you use a lot, you can save it by a name and simply type 
that in instead of the roll's value. This option can be found in the saved rolls 
menu. 
