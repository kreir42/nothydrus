#include "nothydrus.h"

void add_custom_column(char* name, short type, uint_least8_t flags, int lower_limit, int upper_limit){
	char sql_statement[2000], type_name[20];
	switch(type){
		case COLUMN_TYPE_TEXT:
			strcpy(type_name, "TEXT");
			break;
		case COLUMN_TYPE_INTEGER:
			strcpy(type_name, "INTEGER");
			break;
		case COLUMN_TYPE_REAL:
			strcpy(type_name, "REAL");
			break;
	}
	sprintf(sql_statement, "ALTER TABLE files ADD COLUMN %s %s", name, type_name);
	if(flags & COLUMN_NOT_NULL) strcat(sql_statement, " NOT NULL");
	strcat(sql_statement, ";");
	char* sqlite3_error_message;
	if(sqlite3_exec(main_db, sql_statement, NULL, NULL, &sqlite3_error_message)){
		fprintf(stderr, "Error when creating tables and indexes:%s\n", sqlite3_error_message);
		goto end_flag;
	}

	sqlite3_bind_text(add_custom_column_statement, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_int(add_custom_column_statement, 2, type);
	sqlite3_bind_int(add_custom_column_statement, 3, flags);
	sqlite3_bind_int(add_custom_column_statement, 4, lower_limit);
	sqlite3_bind_int(add_custom_column_statement, 5, upper_limit);
	if(sqlite3_step(add_custom_column_statement) != SQLITE_DONE){
		fprintf(stderr, "sqlite3_step(add_custom_column_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
	sqlite3_clear_bindings(add_custom_column_statement);
	if(sqlite3_reset(add_custom_column_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(add_custom_column_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}

	end_flag:
	sqlite3_free(sqlite3_error_message);
}
