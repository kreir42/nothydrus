#include "nothydrus.h"

short run_search(struct search* search){
	search->output_ids.used = 0;

	if(search->sql[0]=='\0') return -1;

	sqlite3_stmt* search_statement;
	if(sqlite3_prepare_v2(main_db, search->sql, -1, &search_statement, NULL)){
		fprintf(stderr, "Error preparing SQL search statement: %s\n", sqlite3_errmsg(main_db));
		return -1;
	}
	int error_code;
	while((error_code=sqlite3_step(search_statement)) == SQLITE_ROW){
		append_id_dynarr(&search->output_ids, sqlite3_column_int64(search_statement, 0));
	}
	if(error_code != SQLITE_DONE){
		fprintf(stderr, "Error executing SQL search statement at row %ld: %s\n", search->output_ids.used, sqlite3_errmsg(main_db));
		return -1;
	}
	sqlite3_finalize(search_statement);
	return 0;
}

void free_search(struct search* search){
	if(search->output_ids.size>0) free(search->output_ids.data);
}
