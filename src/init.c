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

	char* sqlite3_error_message;
	if(sqlite3_exec(main_db,"CREATE TABLE files("
				"id BLOB PRIMARY KEY NOT NULL, "
				"size INT, "
				"flags INT NOT NULL DEFAULT 0, "	//bitfield
				"filepath TEXT);"

				"CREATE TABLE tags("
				"id INT PRIMARY KEY NOT NULL, "
				"name TEXT NOT NULL UNIQUE, "
				"number INT NOT NULL DEFAULT 0);"

				"CREATE UNIQUE INDEX tagsname_index ON tags(name);"

				"CREATE TABLE itemstags("
				"item INT NOT NULL, "
				"tag INT NOT NULL, "
				"PRIMARY KEY(item, tag));"

				,NULL, NULL, &sqlite3_error_message)){
		fprintf(stderr, "Error when creating tables and indexes:\n%s", sqlite3_error_message);
	}

	sqlite3_free(sqlite3_error_message);
	end_program();
}
