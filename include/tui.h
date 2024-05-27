#ifndef TUI_H
#define TUI_H

#include <notcurses/notcurses.h>

extern struct notcurses* nc;

void start_tui(int_least8_t flags);
struct ncplane* display_file(sqlite3_int64 id, int_least8_t flags, struct ncplane* plane);
struct ncplane* display_file_from_filepath(char* filepath, int_least8_t flags, struct ncplane* plane);
#define DISPLAY_FILE_PIXEL	(1<<0)

#endif
