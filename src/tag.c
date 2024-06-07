#include "nothydrus.h"

void add_tag(char* name, char* taggroup_name){
	sqlite3_bind_text(add_tag_statement, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_int64(add_tag_statement, 2, taggroup_id_from_name(taggroup_name));

	if(sqlite3_step(add_tag_statement) != SQLITE_DONE){
		fprintf(stderr, "sqlite3_step(add_tag_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(add_tag_statement);
		sqlite3_reset(add_tag_statement);
		return;
	}
	sqlite3_clear_bindings(add_tag_statement);
	if(sqlite3_reset(add_tag_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(add_tag_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return;
	}
	return;
}
