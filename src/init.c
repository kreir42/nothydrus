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
				"filepath TEXT UNIQUE)"
				"STRICT;"

				"CREATE UNIQUE INDEX files_hashsize_index ON files(hash, size);"
				"CREATE INDEX files_size_index ON files(size);"
				"CREATE INDEX files_filetype_index ON files(filetype);"

				"CREATE TABLE tags("
				"id INTEGER PRIMARY KEY NOT NULL, "
				"name TEXT NOT NULL, "
				"taggroup INTEGER NOT NULL DEFAULT 1, "
				"number INTEGER NOT NULL DEFAULT 0)"
				"STRICT;"

				"CREATE UNIQUE INDEX tagsnametaggroup_index ON tags(name, taggroup);"

				"CREATE TABLE filestags("
				"file INTEGER NOT NULL, "
				"tag INTEGER NOT NULL, "
				"PRIMARY KEY(file, tag))"
				"STRICT;"

				"CREATE TABLE taggroups("
				"id INTEGER PRIMARY KEY NOT NULL, "
				"name TEXT NOT NULL UNIQUE, "
				"number INTEGER NOT NULL DEFAULT 0)"
				"STRICT;"

				"CREATE UNIQUE INDEX taggroupsname_index ON taggroups(name);"

				"CREATE TABLE tagstaggroups("
				"tag INTEGER NOT NULL, "
				"taggroup INTEGER NOT NULL, "
				"PRIMARY KEY(tag, taggroup))"
				"STRICT;"

				"INSERT INTO taggroups(id, name) "
				"VALUES(1, \""DEFAULT_TAGGROUP_NAME"\");"

				"CREATE TABLE custom_columns("
				"name TEXT NOT NULL UNIQUE, "
				"type INTEGER NOT NULL, "
				"flags INTEGER NOT NULL DEFAULT 0, "	//bitfield
				"lower_limit INTEGER, "
				"upper_limit INTEGER)"
				"STRICT;"

				,NULL, NULL, &sqlite3_error_message)){
		fprintf(stderr, "Error when creating tables and indexes:%s\n", sqlite3_error_message);
	}
	prepare_add_custom_column();
	add_custom_column("Rating", COLUMN_TYPE_INTEGER, 0, 0, 10);	//debug, to remove

	sqlite3_free(sqlite3_error_message);
	end_program();
}
