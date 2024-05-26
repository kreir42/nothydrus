#ifndef TUI_H
#define TUI_H

#include <notcurses/notcurses.h>

extern struct notcurses* nc;

void start_tui(int_least8_t flags);

#endif
