#include "nothydrus.h"

sqlite3_int64 id_from_filepath(char* filepath){
	sqlite3_clear_bindings(id_from_filepath_statement);
	if(sqlite3_reset(id_from_filepath_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(id_from_filepath_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return -1;
	}

	sqlite3_bind_text(id_from_filepath_statement, 1, filepath, -1, SQLITE_STATIC);
	if(sqlite3_step(id_from_filepath_statement) != SQLITE_ROW){
		fprintf(stderr, "sqlite3_step(id_from_filepath_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(id_from_filepath_statement);
		sqlite3_reset(id_from_filepath_statement);
		return -1;
	}
	return sqlite3_column_int64(id_from_filepath_statement, 0);
}
