#include "nothydrus.h"

short run_search(struct search* search){
	if(search->input_ids!=NULL) free(search->input_ids);
	if(search->output_ids!=NULL) free(search->output_ids);
	search->input_ids_n = 0;
	search->output_ids_n = 0;

	if(search->sql[0]=='\0') return -1;

	sqlite3_stmt* search_statement;
	if(sqlite3_prepare_v2(main_db, search->sql, -1, &search_statement, NULL)){
		fprintf(stderr, "Error preparing SQL search statement: %s\n", sqlite3_errmsg(main_db));
		return -1;
	}
	int error_code;
	while((error_code=sqlite3_step(search_statement)) == SQLITE_ROW){
		search->output_ids_n++;
	}
	if(error_code != SQLITE_DONE){
		fprintf(stderr, "Error executing SQL search statement at row %d: %s\n", search->output_ids_n, sqlite3_errmsg(main_db));
		return -1;
	}
	sqlite3_finalize(search_statement);
	return 0;
}
