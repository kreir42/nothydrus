#ifndef TUI_H
#define TUI_H

#include <notcurses/notcurses.h>

extern struct notcurses* nc;

void start_tui(int_least8_t flags);
void display_file(sqlite3_int64 id, int_least8_t flags);
void display_file_from_filepath(char* filepath, int_least8_t flags);

#endif
