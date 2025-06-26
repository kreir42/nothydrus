#include "nothydrus.h"

void init(){
	if(mkdir(INIT_DIRECTORY, DEFAULT_DIRECTORY_MODE)){
		perror("Error when trying to create init directory");
		return;
	}
	start_program(START_PROGRAM_INIT);

	chdir(INIT_DIRECTORY);
	//TBD: configs, other files, etc
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
	prepare_add_custom_column();
	end_program();
}
