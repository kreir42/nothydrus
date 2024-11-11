#include "nothydrus.h"

void search_tags(struct id_dynarr* dynarr, char* tag_search){
	char* tag_name = tag_search;
	while(*tag_name==' ') tag_name++;	//skip begginning whitespace
	for(unsigned short i=strlen(tag_name)-1; i>0; i--){
		if(tag_name[i]==' ') tag_name[i]='\0';	//remove trailing whitespace
		else break;
	}

	unsigned int tag_name_len = strlen(tag_name);
	char* tag_name_search = malloc((tag_name_len+3)*sizeof(char));
	if(tag_name[0]=='"' && tag_name[tag_name_len-1]=='"'){
		tag_name++;
		sprintf(tag_name_search, "%s", tag_name);
		tag_name_search[strlen(tag_name_search)-1] = '\0';
	}else{
		sprintf(tag_name_search, "%%%s%%", tag_name);
	}

	dynarr->used = 0;
	sqlite3_clear_bindings(search_tags_statement);
	if(sqlite3_reset(search_tags_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(search_tags_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
	sqlite3_bind_text(search_tags_statement, 1, tag_name_search, -1, SQLITE_STATIC);
	int error_code;
	while((error_code=sqlite3_step(search_tags_statement)) == SQLITE_ROW){
		append_id_dynarr(dynarr, sqlite3_column_int64(search_tags_statement, 0));
	}
	if(error_code != SQLITE_DONE){
		fprintf(stderr, "Error executing search_tags_statement at row %ld: %s\n", dynarr->used, sqlite3_errmsg(main_db));
	}
	free(tag_name_search);
}
