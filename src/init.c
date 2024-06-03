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
				"id INTEGER PRIMARY KEY NOT NULL, "
				"hash BLOB, "
				"size INTEGER, "
				"filetype INTEGER NOT NULL, "
				"flags INTEGER NOT NULL DEFAULT 0, "	//bitfield
				"filepath TEXT);"

				"CREATE UNIQUE INDEX files_hashsize_index ON files(hash, size);"

				"CREATE TABLE tags("
				"id INTEGER PRIMARY KEY NOT NULL, "
				"name TEXT NOT NULL, "
				"taggroup INT DEFAULT NULL, "
				"number INTEGER NOT NULL DEFAULT 0);"

				"CREATE UNIQUE INDEX tagsnametaggroup_index ON tags(name, taggroup);"

				"CREATE TABLE itemstags("
				"item INTEGER NOT NULL, "
				"tag INTEGER NOT NULL, "
				"PRIMARY KEY(item, tag));"

				"CREATE TABLE taggroups("
				"id INTEGER PRIMARY KEY NOT NULL, "
				"name TEXT NOT NULL UNIQUE, "
				"number INTEGER NOT NULL DEFAULT 0);"

				"CREATE UNIQUE INDEX taggroupsname_index ON taggroups(name);"

				"CREATE TABLE tagstaggroups("
				"tag INTEGER NOT NULL, "
				"taggroup INTEGER NOT NULL, "
				"PRIMARY KEY(tag, taggroup));"

				,NULL, NULL, &sqlite3_error_message)){
		fprintf(stderr, "Error when creating tables and indexes:\n%s", sqlite3_error_message);
	}

	sqlite3_free(sqlite3_error_message);
	end_program();
}
