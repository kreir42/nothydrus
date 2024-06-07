#include "nothydrus.h"

char* hash_from_id(sqlite3_int64 id){
	sqlite3_clear_bindings(hash_from_id_statement);
	if(sqlite3_reset(hash_from_id_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(hash_from_id_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return NULL;
	}

	sqlite3_bind_int64(hash_from_id_statement, 1, id);
	if(sqlite3_step(hash_from_id_statement) != SQLITE_ROW){
		fprintf(stderr, "sqlite3_step(hash_from_id_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(hash_from_id_statement);
		sqlite3_reset(hash_from_id_statement);
		return NULL;
	}
	return sqlite3_column_blob(hash_from_id_statement, 0);
}
