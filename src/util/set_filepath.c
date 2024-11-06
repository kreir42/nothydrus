#include "nothydrus.h"

void set_filepath(sqlite3_int64 id, char* filepath){
	sqlite3_clear_bindings(set_filepath_statement);
	if(sqlite3_reset(set_filepath_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(set_filepath_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return;
	}
	sqlite3_bind_text(set_filepath_statement, 1, filepath, -1, SQLITE_STATIC);
	sqlite3_bind_int64(set_filepath_statement, 2, id);
	if(sqlite3_step(set_filepath_statement) != SQLITE_DONE){
		fprintf(stderr, "sqlite3_step(set_filepath_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
	sqlite3_clear_bindings(set_filepath_statement);
	if(sqlite3_reset(set_filepath_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(set_filepath_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
}
