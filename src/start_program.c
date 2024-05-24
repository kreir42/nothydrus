#include "nothydrus.h"

sqlite3_stmt* add_file_statement;

static void prepare_add_file(){
	if(sqlite3_prepare_v3(main_db,
				"INSERT INTO files("
				"id, size, flags, filepath) "
				"VALUES(?, ?, ?, ?);"
				, -1, SQLITE_PREPARE_PERSISTENT, &add_file_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing add_file_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

void start_program(int_least8_t flags){
	//assumes it starts in the program's root directory
	if(sqlite3_open(INIT_DIRECTORY"/"MAIN_DATABASE_NAME, &main_db)){
		fprintf(stderr, "Error opening main database: %s\n", sqlite3_errmsg(main_db));
		return;
	}
	if(flags & START_PROGRAM_INIT){
		return;
	}
	if(flags & START_PROGRAM_ADD_FILES){
		prepare_add_file();
		return;
	}
	prepare_add_file();
}

void end_program(){
	sqlite3_finalize(add_file_statement);

	sqlite3_close(main_db);

}
