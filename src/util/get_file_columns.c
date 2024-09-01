#include "nothydrus.h"

void get_file_columns(sqlite3_int64 file_id){
	sqlite3_clear_bindings(get_file_columns_statement);
	if(sqlite3_reset(get_file_columns_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(get_file_columns_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return;
	}

	sqlite3_bind_int64(get_file_columns_statement, 1, file_id);
	if(sqlite3_step(get_file_columns_statement) != SQLITE_ROW){
		fprintf(stderr, "sqlite3_step(get_file_columns_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(get_file_columns_statement);
		sqlite3_reset(get_file_columns_statement);
	}
}
