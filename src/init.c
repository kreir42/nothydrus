#include "nothydrus.h"

void init(){
	if(mkdir(INIT_DIRECTORY, DEFAULT_DIRECTORY_MODE)){
		perror("Error when trying to create init directory");
		return;
	}
	chdir(INIT_DIRECTORY);

	sqlite3* main_db;
	if(sqlite3_open(MAIN_DATABASE_NAME, &main_db)){
		fprintf(stderr, "Error creating main database: %s\n", sqlite3_errmsg(main_db));
		return;
	}

	char* sqlite3_error_message;
	if(sqlite3_exec(main_db,"CREATE TABLE files("
				"id INT PRIMARY KEY NOT NULL, "
				"size INT, "
				"flags INT NOT NULL DEFAULT 0, "	//bitfield
				"dir TEXT, "
				"filename TEXT);"

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
	sqlite3_close(main_db);
	chdir("..");
}
