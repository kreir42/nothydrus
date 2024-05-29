#include "nothydrus.h"

sqlite3_stmt* add_file_statement;
sqlite3_stmt* filepath_from_id_statement;

static void prepare_add_file(){
	if(sqlite3_prepare_v3(main_db,
				"INSERT INTO files("
				"hash, size, filetype, flags, filepath) "
				"VALUES(?, ?, ?, ?, ?);"
				, -1, SQLITE_PREPARE_PERSISTENT, &add_file_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing add_file_statement: %s\n", sqlite3_errmsg(main_db));
	}
}

static void prepare_filepath_from_id(){
	if(sqlite3_prepare_v3(main_db,
				"SELECT filepath FROM files "
				"WHERE id = ?;"
				, -1, SQLITE_PREPARE_PERSISTENT, &filepath_from_id_statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing filepath_from_id_statement: %s\n", sqlite3_errmsg(main_db));
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
	if(flags & (START_PROGRAM_SQL_SEARCH|START_PROGRAM_DISPLAY)){
		prepare_filepath_from_id();
		return;
	}
	prepare_add_file();
	prepare_filepath_from_id();
}

void end_program(){
	sqlite3_finalize(add_file_statement);
	sqlite3_finalize(filepath_from_id_statement);

	sqlite3_close(main_db);

}
