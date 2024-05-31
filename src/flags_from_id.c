#include "nothydrus.h"

int flags_from_id(sqlite3_int64 id){
	sqlite3_clear_bindings(flags_from_id_statement);
	if(sqlite3_reset(flags_from_id_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(flags_from_id_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return ~0;
	}
	sqlite3_bind_int64(flags_from_id_statement, 1, id);
	if(sqlite3_step(flags_from_id_statement) != SQLITE_ROW){
		fprintf(stderr, "sqlite3_step(flags_from_id_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(flags_from_id_statement);
		sqlite3_reset(flags_from_id_statement);
		return ~0;
	}
	return sqlite3_column_int(flags_from_id_statement, 0);
}
