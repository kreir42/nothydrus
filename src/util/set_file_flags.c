#include "nothydrus.h"

void set_file_flags(sqlite3_int64 id, int_least8_t flags){
	sqlite3_clear_bindings(set_file_flags_statement);
	if(sqlite3_reset(set_file_flags_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(set_file_flags_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return;
	}
	sqlite3_bind_int(set_file_flags_statement, 1, flags);
	sqlite3_bind_int64(set_file_flags_statement, 2, id);
	if(sqlite3_step(set_file_flags_statement) != SQLITE_DONE){
		fprintf(stderr, "sqlite3_step(set_file_flags_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
	sqlite3_clear_bindings(set_file_flags_statement);
	if(sqlite3_reset(set_file_flags_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(set_file_flags_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
}

void add_flag_to_file(sqlite3_int64 id, int_least8_t flag){
	sqlite3_clear_bindings(add_flag_to_file_statement);
	if(sqlite3_reset(add_flag_to_file_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(add_flag_to_file_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return;
	}
	sqlite3_bind_int(add_flag_to_file_statement, 1, flag);
	sqlite3_bind_int64(add_flag_to_file_statement, 2, id);
	if(sqlite3_step(add_flag_to_file_statement) != SQLITE_DONE){
		fprintf(stderr, "sqlite3_step(add_flag_to_file_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
	sqlite3_clear_bindings(add_flag_to_file_statement);
	if(sqlite3_reset(add_flag_to_file_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(add_flag_to_file_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
}

void remove_flag_from_file(sqlite3_int64 id, int_least8_t flag){
	sqlite3_clear_bindings(remove_flag_from_file_statement);
	if(sqlite3_reset(remove_flag_from_file_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(remove_flag_from_file_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return;
	}
	sqlite3_bind_int(remove_flag_from_file_statement, 1, flag);
	sqlite3_bind_int64(remove_flag_from_file_statement, 2, id);
	if(sqlite3_step(remove_flag_from_file_statement) != SQLITE_DONE){
		fprintf(stderr, "sqlite3_step(remove_flag_from_file_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
	sqlite3_clear_bindings(remove_flag_from_file_statement);
	if(sqlite3_reset(remove_flag_from_file_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(remove_flag_from_file_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
}
