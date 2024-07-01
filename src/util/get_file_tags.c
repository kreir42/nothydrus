#include "nothydrus.h"

void get_file_tags(sqlite3_int64 file_id, struct id_dynarr* dynarr){
	dynarr->used = 0;
	sqlite3_clear_bindings(get_file_tags_statement);
	if(sqlite3_reset(get_file_tags_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(get_file_tags_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
	sqlite3_bind_int64(get_file_tags_statement, 1, file_id);
	int error_code;
	while((error_code=sqlite3_step(get_file_tags_statement)) == SQLITE_ROW){
		append_id_dynarr(dynarr, sqlite3_column_int64(get_file_tags_statement, 0));
	}
	if(error_code != SQLITE_DONE){
		fprintf(stderr, "Error executing get_file_tags_statement at row %ld: %s\n", dynarr->used, sqlite3_errmsg(main_db));
	}
}
