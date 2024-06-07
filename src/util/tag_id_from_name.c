#include "nothydrus.h"

sqlite3_int64 tag_id_from_name(char* name, sqlite3_int64 taggroup){
	if(taggroup<1) taggroup = 1;
	sqlite3_clear_bindings(tag_id_from_name_statement);
	if(sqlite3_reset(tag_id_from_name_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(tag_id_from_name_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return -1;
	}

	sqlite3_bind_text(tag_id_from_name_statement, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_int64(tag_id_from_name_statement, 2, taggroup);
	if(sqlite3_step(tag_id_from_name_statement) != SQLITE_ROW){
		fprintf(stderr, "sqlite3_step(tag_id_from_name_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(tag_id_from_name_statement);
		sqlite3_reset(tag_id_from_name_statement);
		return -1;
	}
	return sqlite3_column_int64(tag_id_from_name_statement, 0);
}
