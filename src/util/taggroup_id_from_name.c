#include "nothydrus.h"

sqlite3_int64 taggroup_id_from_name(char* name){
	sqlite3_clear_bindings(taggroup_id_from_name_statement);
	if(sqlite3_reset(taggroup_id_from_name_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(taggroup_id_from_name_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return -1;
	}

	sqlite3_bind_text(taggroup_id_from_name_statement, 1, name, -1, SQLITE_STATIC);
	if(sqlite3_step(taggroup_id_from_name_statement) != SQLITE_ROW){
		fprintf(stderr, "sqlite3_step(taggroup_id_from_name_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(taggroup_id_from_name_statement);
		sqlite3_reset(taggroup_id_from_name_statement);
		return -1;
	}
	return sqlite3_column_int64(taggroup_id_from_name_statement, 0);
}
