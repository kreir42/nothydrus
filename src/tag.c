#include "nothydrus.h"

void add_tag(char* name, char* taggroup_name){
	sqlite3_bind_text(add_tag_statement, 1, name, -1, SQLITE_STATIC);
	sqlite3_int64 taggroup_id;
	if(taggroup_name==NULL) taggroup_id = 1;
	else taggroup_id = taggroup_id_from_name(taggroup_name);
	if(taggroup_id==-1){
		fprintf(stderr, "Error in add_tag: taggroup_id_from_name(taggroup_name) returned -1, probably meaning taggroup couldnt be found\n");
		return;
	}
	sqlite3_bind_int64(add_tag_statement, 2, taggroup_id);

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

void add_taggroup(char* name){
	sqlite3_bind_text(add_taggroup_statement, 1, name, -1, SQLITE_STATIC);

	if(sqlite3_step(add_taggroup_statement) != SQLITE_DONE){
		fprintf(stderr, "sqlite3_step(add_taggroup_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(add_taggroup_statement);
		sqlite3_reset(add_taggroup_statement);
		return;
	}
	sqlite3_clear_bindings(add_taggroup_statement);
	if(sqlite3_reset(add_taggroup_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(add_taggroup_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return;
	}
	return;
}

void tag(sqlite3_int64 file_id, sqlite3_int64 tag_id){
	sqlite3_bind_int64(tag_statement, 1, file_id);
	sqlite3_bind_int64(tag_statement, 2, tag_id);

	if(sqlite3_step(tag_statement) != SQLITE_DONE){
		fprintf(stderr, "sqlite3_step(tag_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(tag_statement);
		sqlite3_reset(tag_statement);
		return;
	}
	sqlite3_clear_bindings(tag_statement);
	if(sqlite3_reset(tag_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(tag_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return;
	}
	return;
}

void untag(sqlite3_int64 file_id, sqlite3_int64 tag_id){
	sqlite3_bind_int64(untag_statement, 1, file_id);
	sqlite3_bind_int64(untag_statement, 2, tag_id);

	if(sqlite3_step(untag_statement) != SQLITE_DONE){
		fprintf(stderr, "sqlite3_step(untag_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(untag_statement);
		sqlite3_reset(untag_statement);
		return;
	}
	sqlite3_clear_bindings(untag_statement);
	if(sqlite3_reset(untag_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(untag_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return;
	}
	return;
}
