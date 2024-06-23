#include "nothydrus.h"

char* tag_name_from_id(sqlite3_int64 id, sqlite3_int64* taggroup){
	sqlite3_clear_bindings(tag_name_from_id_statement);
	if(sqlite3_reset(tag_name_from_id_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(tag_name_from_id_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return NULL;
	}

	sqlite3_bind_int64(tag_name_from_id_statement, 1, id);
	if(sqlite3_step(tag_name_from_id_statement) != SQLITE_ROW){
		fprintf(stderr, "sqlite3_step(tag_name_from_id_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(tag_name_from_id_statement);
		sqlite3_reset(tag_name_from_id_statement);
		return NULL;
	}
	if(taggroup!=NULL) *taggroup = sqlite3_column_int64(tag_name_from_id_statement, 1);
	return sqlite3_column_text(tag_name_from_id_statement, 0);
}

char* tag_fullname_from_id(sqlite3_int64 id){
	sqlite3_clear_bindings(tag_fullname_from_id_statement);
	if(sqlite3_reset(tag_fullname_from_id_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(tag_fullname_from_id_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return NULL;
	}

	sqlite3_bind_int64(tag_fullname_from_id_statement, 1, id);
	if(sqlite3_step(tag_fullname_from_id_statement) != SQLITE_ROW){
		fprintf(stderr, "sqlite3_step(tag_fullname_from_id_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(tag_fullname_from_id_statement);
		sqlite3_reset(tag_fullname_from_id_statement);
		return NULL;
	}
	return sqlite3_column_text(tag_fullname_from_id_statement, 0);
}
