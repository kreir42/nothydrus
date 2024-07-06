#ifndef TUI_H
#define TUI_H

#include <notcurses/notcurses.h>

struct tui_options{
	unsigned long search_limit;
	unsigned char shortcuts_n;
	struct shortcut{
		uint32_t key;
		unsigned char type;
		sqlite3_int64 id;
	}* shortcuts;
};
#define SHORTCUT_TYPE_TAG_FILE 1
#define SHORTCUT_TYPE_UNTAG_FILE 2
#define SHORTCUT_TYPE_TAG_UNTAG_FILE 3

extern struct notcurses* nc;
extern struct tui_options tui_options;

void start_tui(int_least8_t flags, void* data);
#define START_TUI_DISPLAY	(1<<0)

void fullscreen_display(struct search* search);
unsigned short chooser(struct ncplane* parent_plane, char** options, unsigned short initial_value);
char* input_reader(struct ncplane* parent_plane, int y, int x, int h, int w);	//TBD initial value

struct ncplane* display_file(sqlite3_int64 id, int_least8_t flags, struct ncplane* plane);
struct ncplane* display_file_from_filepath(char* filepath, int_least8_t flags, struct ncplane* plane);
#define DISPLAY_FILE_FAST	(1<<0)
#define DISPLAY_FILE_MPV	(1<<1)
#define DISPLAY_FILE_EXTERNAL	(1<<2)
void reset_display_plane(struct ncplane* plane);

void file_tag_tui(sqlite3_int64 id);

#endif
