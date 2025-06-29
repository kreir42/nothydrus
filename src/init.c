#include "nothydrus.h"
#include "tui.h"

void init(){
	if(mkdir(INIT_DIRECTORY, DEFAULT_DIRECTORY_MODE)){
		perror("Error when trying to create init directory");
		return;
	}
	chdir(INIT_DIRECTORY);
	if(sqlite3_open(MAIN_DATABASE_NAME, &main_db)){
		fprintf(stderr, "Error creating main database: %s\n", sqlite3_errmsg(main_db));
		return;
	}

	//add default config file
	struct tui_options default_tui_options = {
		.search_order_by = none,
		.search_descending = 0,
		.search_limit = 0,
		.shortcuts_n = 6,
		.shortcuts = malloc(sizeof(struct shortcut)*6)
	};
	default_tui_options.shortcuts[0] = (struct shortcut){.key = NCKEY_RIGHT, .type = SHORTCUT_TYPE_FULLSCREEN_NEXT};
	default_tui_options.shortcuts[1] = (struct shortcut){.key = NCKEY_LEFT, .type = SHORTCUT_TYPE_FULLSCREEN_PREV};
	default_tui_options.shortcuts[2] = (struct shortcut){.key = 't', .type = SHORTCUT_TYPE_FULLSCREEN_TAG};
	default_tui_options.shortcuts[3] = (struct shortcut){.key = 'o', .type = SHORTCUT_TYPE_FULLSCREEN_OPTIONS};
	default_tui_options.shortcuts[4] = (struct shortcut){.key = ':', .type = SHORTCUT_TYPE_FULLSCREEN_COMMAND};
	default_tui_options.shortcuts[5] = (struct shortcut){.key = 'q', .type = SHORTCUT_TYPE_FULLSCREEN_QUIT};
	tui_options = default_tui_options;
	save_tui_options("tui_options");
	chdir("..");

	char* sqlite3_error_message = NULL;
	if(sqlite3_exec(main_db,"CREATE TABLE files("
				"id INTEGER PRIMARY KEY NOT NULL, "
				"hash BLOB, "
				"size INTEGER, "
				"filetype INTEGER NOT NULL, "
				"flags INTEGER NOT NULL DEFAULT 0, "	//bitfield
				"filepath TEXT UNIQUE)"
				"STRICT;"

				"CREATE UNIQUE INDEX files_hashsize_index ON files(hash, size);"
				"CREATE INDEX files_size_index ON files(size);"
				"CREATE INDEX files_filetype_index ON files(filetype);"

				"CREATE TABLE tags("
				"id INTEGER PRIMARY KEY NOT NULL, "
				"name TEXT NOT NULL, "
				"number INTEGER NOT NULL DEFAULT 0)"
				"STRICT;"

				"CREATE TABLE filestags("
				"file INTEGER NOT NULL, "
				"tag INTEGER NOT NULL, "
				"PRIMARY KEY(file, tag))"
				"STRICT;"

				"CREATE TABLE custom_columns("
				"id INTEGER PRIMARY KEY NOT NULL, "
				"name TEXT NOT NULL UNIQUE, "
				"flags INTEGER NOT NULL DEFAULT 0, "	//bitfield
				"lower_limit INTEGER, "
				"upper_limit INTEGER)"
				"STRICT;"

				,NULL, NULL, &sqlite3_error_message)){
		fprintf(stderr, "Error when creating tables and indexes:%s\n", sqlite3_error_message);
		sqlite3_free(sqlite3_error_message);
	}
	end_program();
}
