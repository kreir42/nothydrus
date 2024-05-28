#ifndef TUI_H
#define TUI_H

#include <notcurses/notcurses.h>

extern struct notcurses* nc;

void start_tui(int_least8_t flags);

struct ncplane* new_search_plane(struct search* search_to_copy);
void search_plane(struct ncplane* plane);
void free_search_plane(struct ncplane* plane);

struct ncplane* display_file(sqlite3_int64 id, int_least8_t flags, struct ncplane* plane);
struct ncplane* display_file_from_filepath(char* filepath, int_least8_t flags, struct ncplane* plane);
#define DISPLAY_FILE_FAST	(1<<0)
void reset_display_plane(struct ncplane* plane);

#endif
