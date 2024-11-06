#include "nothydrus.h"

void search_file_from_hash(char* hash, unsigned long filesize, struct id_dynarr* dynarr){
	dynarr->used = 0;
	sqlite3_clear_bindings(search_file_from_hash_statement);
	if(sqlite3_reset(search_file_from_hash_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(search_file_from_hash_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
	sqlite3_bind_int64(search_file_from_hash_statement, 1, filesize);
	sqlite3_bind_blob64(search_file_from_hash_statement, 2, hash, HASH_SIZE*sizeof(char), SQLITE_STATIC);
	int error_code;
	while((error_code=sqlite3_step(search_file_from_hash_statement)) == SQLITE_ROW){
		append_id_dynarr(dynarr, sqlite3_column_int64(search_file_from_hash_statement, 0));
	}
	if(error_code != SQLITE_DONE){
		fprintf(stderr, "Error executing search_file_from_hash_statement at row %ld: %s\n", dynarr->used, sqlite3_errmsg(main_db));
	}
}
