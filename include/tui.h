#ifndef TUI_H
#define TUI_H

#include <notcurses/notcurses.h>

struct tui_options{
	enum order_by_enum search_order_by;
	char search_descending;
	unsigned long search_limit;

	unsigned short modes_n; //other than the default mode
	char** modes; //NULL-terminated array

	unsigned short shortcuts_n;
	struct shortcut{
		uint32_t key;
		unsigned char type;
		unsigned short mode;
		sqlite3_int64 id;
		int value;
		char* string;
	}* shortcuts;
};
#define SHORTCUT_TYPE_TAG_FILE 0
#define SHORTCUT_TYPE_UNTAG_FILE 1
#define SHORTCUT_TYPE_TAG_UNTAG_FILE 2
#define SHORTCUT_TYPE_CUSTOM_COLUMN_INCREASE 3
#define SHORTCUT_TYPE_CUSTOM_COLUMN_DECREASE 4
#define SHORTCUT_TYPE_CUSTOM_COLUMN_RESET 5
#define SHORTCUT_TYPE_CUSTOM_COLUMN_SET 6
#define SHORTCUT_TYPE_EXTERNAL_COMMAND 7
#define SHORTCUT_TYPE_FULLSCREEN_NEXT 8
#define SHORTCUT_TYPE_FULLSCREEN_PREV 9
#define SHORTCUT_TYPE_FULLSCREEN_TAG 10
#define SHORTCUT_TYPE_FULLSCREEN_OPTIONS 11
#define SHORTCUT_TYPE_FULLSCREEN_COMMAND 12
#define SHORTCUT_TYPE_FULLSCREEN_QUIT 13
#define SHORTCUT_TYPE_FULLSCREEN_CHOOSE_MODE 14
void options_tui();
short save_tui_options(char* name);
void load_tui_options(char* name);
void delete_shortcut(unsigned short index);

extern struct notcurses* nc;
extern struct tui_options tui_options;

void start_tui(int_least8_t flags, void* data);
#define START_TUI_DISPLAY	(1<<0)

void fullscreen_display(struct search* search);
short chooser(struct ncplane* parent_plane, char** options, short initial_value);
uint_least8_t multiple_chooser(struct ncplane* parent_plane, char** options, uint_least8_t initial_value);
char* input_reader(struct ncplane* parent_plane, int y, int x, int h, int w, const char* pre_text, const char* post_text, const char* prompt, bool box);
char* centered_input_reader(struct ncplane* parent_plane, int h, int w, const char* pre_text, const char* post_text, const char* prompt, bool box);

struct ncplane* display_file(sqlite3_int64 id, int_least8_t flags, struct ncplane* plane);
struct ncplane* display_file_from_filepath(char* filepath, int_least8_t flags, struct ncplane* plane);
#define DISPLAY_FILE_FAST	(1<<0)
void reset_display_plane(struct ncplane* plane);
void external_command_on_file(sqlite3_int64 id, char* command);

void file_tag_tui(sqlite3_int64 id);

sqlite3_int64 search_tag_tui();
#endif
