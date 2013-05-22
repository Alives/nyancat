#ifndef __COLORS_H
#define __COLORS_H

#define TOTAL_TERMINAL_TYPES 6

/*
 * keys[0] = xterm
 * keys[1] = linux
 * keys[2] = vt220
 * keys[3] = fallback
 * keys[4] = rxvt
 * keys[5] = default
 *
 * color[key][0] = Blue background
 * color[key][1] = White stars
 * color[key][2] = Black border
 * color[key][3] = Tan poptart
 * color[key][4] = Pink poptart
 * color[key][5] = Red poptart
 * color[key][6] = Red rainbow
 * color[key][7] = Orange rainbow
 * color[key][8] = Yellow Rainbow
 * color[key][9] = Green rainbow
 * color[key][10] = Light blue rainbow
 * color[key][11] = Dark blue rainbow
 * color[key][12] = Gray cat face
 * color[key][13] = Pink cheeks
 * color[key][14] = White duration time
 * color[key][15] = xterm terminal
 */


static char * colors[TOTAL_TERMINAL_TYPES][17] = { {
    "\033[48;5;17m",
    "\033[48;5;15m",
    "\033[48;5;0m",
    "\033[48;5;230m",
    "\033[48;5;175m",
    "\033[48;5;162m",
    "\033[48;5;9m",
    "\033[48;5;202m",
    "\033[48;5;11m",
    "\033[48;5;10m",
    "\033[48;5;33m",
    "\033[48;5;19m",
    "\033[48;5;8m",
    "\033[48;5;175m",
    "\033[1;37m",
    " ",
    "xterm"
  }, {
    "\033[25;44m",
    "\033[5;47m",
    "\033[25;40m",
    "\033[5;47m",
    "\033[5;45m",
    "\033[5;41m",
    "\033[5;41m",
    "\033[25;43m",
    "\033[5;43m",
    "\033[5;42m",
    "\033[25;44m",
    "\033[5;44m",
    "\033[5;40m",
    "\033[5;45m",
    "\033[1;37m",
    " ",
    "linux",
  }, {
    "::",
    "@@",
    "  ",
    "##",
    "??",
    "<>",
    "##",
    "==",
    "--",
    "++",
    "~~",
    "$$",
    ";;",
    "()",
    "",
    " ",
    "vt220",
  }, {
    "\033[0;34;44m",
    "\033[1;37;47m",
    "\033[0;30;40m",
    "\033[1;37;47m",
    "\033[1;35;45m",
    "\033[1;31;41m",
    "\033[1;31;41m",
    "\033[0;33;43m",
    "\033[1;33;43m",
    "\033[1;32;42m",
    "\033[1;34;44m",
    "\033[0;34;44m",
    "\033[1;30;40m",
    "\033[1;35;45m",
    "\033[1;37m",
    "\342\226\210",
    "fallback",
  }, {
    "\033[25;44m",
    "\033[5;47m",
    "\033[25;40m",
    "\033[5;47m",
    "\033[5;45m",
    "\033[5;41m",
    "\033[5;41m",
    "\033[25;43m",
    "\033[5;43m",
    "\033[5;42m",
    "\033[25;44m",
    "\033[5;44m",
    "\033[5;40m",
    "\033[5;45m",
    "\033[1;37m",
    " ",
    "rxvt",
  }, {
    "\033[104m",
    "\033[107m",
    "\033[40m",
    "\033[47m",
    "\033[105m",
    "\033[101m",
    "\033[101m",
    "\033[43m",
    "\033[103m",
    "\033[102m",
    "\033[104m",
    "\033[44m",
    "\033[100m",
    "\033[105m",
    "\033[1;37m",
    " ",
    "default"} };
#endif
