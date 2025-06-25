#include "nothydrus.h"

void add_custom_column(char* name, uint_least8_t flags, int lower_limit, int upper_limit){
	char sql_statement[2000];
	sprintf(sql_statement, "ALTER TABLE files ADD COLUMN \"%s\" INTEGER", name);
	if(flags & COLUMN_NOT_NULL) strcat(sql_statement, " NOT NULL");
	strcat(sql_statement, ";");
	char* sqlite3_error_message;
	if(sqlite3_exec(main_db, sql_statement, NULL, NULL, &sqlite3_error_message)){
		fprintf(stderr, "sqlite3_exec(%s) in add_custom_column returned an error:%s\n", sql_statement, sqlite3_error_message);
		goto end_flag;
	}

	sqlite3_bind_text(add_custom_column_statement, 1, name, -1, SQLITE_STATIC);
	sqlite3_bind_int(add_custom_column_statement, 2, flags);
	sqlite3_bind_int(add_custom_column_statement, 3, lower_limit);
	sqlite3_bind_int(add_custom_column_statement, 4, upper_limit);
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

//get all custom columns data from the custom_columns SQL table into the custom_columns global variable
void get_custom_columns(){
	sqlite3_stmt* statement;
	if(sqlite3_prepare_v3(main_db,
				"SELECT COUNT(1) FROM custom_columns;"
				, -1, 0, &statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing count statement in get_custom_columns: %s\n", sqlite3_errmsg(main_db));
	}
	if(sqlite3_step(statement) != SQLITE_ROW){
		fprintf(stderr, "sqlite3_step(statement) returned an error for count statement in get_custom_columns: %s\n", sqlite3_errmsg(main_db));
		return;
	}
	custom_columns_n = sqlite3_column_int(statement, 0);
	sqlite3_finalize(statement);
	if(sqlite3_prepare_v3(main_db,
				"SELECT * FROM custom_columns;"
				, -1, 0, &statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing statement in get_custom_columns: %s\n", sqlite3_errmsg(main_db));
	}
	custom_columns = realloc(custom_columns, custom_columns_n*sizeof(struct custom_column));
	int error_code;
	unsigned int row_n = 0;
	while((error_code=sqlite3_step(statement)) == SQLITE_ROW){
		strcpy(custom_columns[row_n].name, (char* )sqlite3_column_text(statement, 1));
		custom_columns[row_n].flags = sqlite3_column_int(statement, 2);
		custom_columns[row_n].lower_limit = sqlite3_column_int(statement, 3);
		custom_columns[row_n].upper_limit = sqlite3_column_int(statement, 4);
		row_n++;
	}
	if(error_code != SQLITE_DONE){
		fprintf(stderr, "Error executing statement in get_custom_columns at row %d of %d: %s\n", row_n, custom_columns_n, sqlite3_errmsg(main_db));
	}
	sqlite3_finalize(statement);
}

void set_custom_column_value(sqlite3_int64 file_id, unsigned short custom_column_id, void* value){
	char statement_string[100+CUSTOM_COLUMN_NAME_SIZE];
	strcpy(statement_string, "UPDATE files SET ");
	strcat(statement_string, custom_columns[custom_column_id].name);
	strcat(statement_string, " = ? WHERE id = ?");
	sqlite3_stmt* statement;
	if(sqlite3_prepare_v3(main_db,
				statement_string
				, -1, 0, &statement, NULL) != SQLITE_OK){
		fprintf(stderr, "Error preparing statement in set_custom_column_value: %s\n", sqlite3_errmsg(main_db));
	}
	sqlite3_bind_int(statement, 1, *(int*)value);
	sqlite3_bind_int64(statement, 2, file_id);
	if(sqlite3_step(statement) != SQLITE_DONE){
		fprintf(stderr, "Error executing statement in set_custom_column_value: %s\n", sqlite3_errmsg(main_db));
	}
	sqlite3_finalize(statement);
}
