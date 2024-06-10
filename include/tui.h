#ifndef TUI_H
#define TUI_H

#include <notcurses/notcurses.h>

extern struct notcurses* nc;
extern struct ncplane* search_planes[MAX_SEARCH_PLANES];

void start_tui(int_least8_t flags, void* data);
#define START_TUI_DISPLAY	(1<<0)

struct ncplane* new_search_plane(struct search* search_to_copy);
void search_plane(struct ncplane* plane);
void free_search_plane(struct ncplane* plane);

void fullscreen_display(struct search* search);

struct ncplane* display_file(sqlite3_int64 id, int_least8_t flags, struct ncplane* plane);
struct ncplane* display_file_from_filepath(char* filepath, int_least8_t flags, struct ncplane* plane);
#define DISPLAY_FILE_FAST	(1<<0)
#define DISPLAY_FILE_MPV	(1<<1)
#define DISPLAY_FILE_EXTERNAL	(1<<2)
void reset_display_plane(struct ncplane* plane);

#endif
