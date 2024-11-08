#include "nothydrus.h"

short run_search(struct search* search){
	search->output_ids.used = 0;

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

static void add_where_clause(char* sql, char* clause){
	if(strcmp(&sql[strlen(sql)-6], " WHERE")!=0) strcat(sql, " AND");
	strcat(sql, clause);
}

short compose_search_sql(struct search* search){
	strcpy(search->sql, "SELECT id FROM files");
	if(search->min_size || search->max_size || search->filetypes || search->include_tags_n>0 || search->exclude_tags_n>0 || search->or_tag_elements_n>0){
		strcat(search->sql, " WHERE");
		char where_clause[WHERE_CLAUSE_SIZE];
		if(search->min_size){
			sprintf(where_clause, " size>=%lu", search->min_size);
			add_where_clause(search->sql, where_clause);
		}
		if(search->max_size){
			sprintf(where_clause, " size<=%lu", search->max_size);
			add_where_clause(search->sql, where_clause);
		}
		if(search->filetypes){
			sprintf(where_clause, " filetype&%u", search->filetypes);
			add_where_clause(search->sql, where_clause);
		}
		if(search->include_tags_n>0 || search->exclude_tags_n>0 || search->or_tag_elements_n>0){
			char tag_clause[TAG_CLAUSE_SIZE];
			if(search->include_tags_n>0){
				strcpy(tag_clause, " id IN(SELECT file FROM filestags WHERE tag");
				strcat(tag_clause, " IN(");
				for(unsigned short i=0; i<search->include_tags_n; i++){
					if(i>0) strcat(tag_clause, ", ");
					sprintf(where_clause, "%llu", search->include_tags[i]);
					strcat(tag_clause, where_clause);
				}
				strcat(tag_clause, ")");
				if(search->include_tags_n>1){
					sprintf(where_clause, " GROUP BY file HAVING COUNT(1)=%d", search->include_tags_n);
					strcat(tag_clause, where_clause);
				}
				if(search->exclude_tags_n>0) strcat(tag_clause, " EXCEPT SELECT file FROM filestags WHERE tag IN(");
			}
			if(search->exclude_tags_n>0){
				if(search->include_tags_n==0) strcpy(tag_clause, " id NOT IN(SELECT file FROM filestags WHERE tag IN(");
				for(unsigned short i=0; i<search->exclude_tags_n; i++){
					if(i>0) strcat(tag_clause, ", ");
					sprintf(where_clause, "%llu", search->exclude_tags[i]);
					strcat(tag_clause, where_clause);
				}
				strcat(tag_clause, ")");
			}
			if(search->or_tag_elements_n>0){
				if(search->include_tags_n>0) strcat(tag_clause, " INTERSECT SELECT file FROM filestags WHERE tag");
				else if(search->exclude_tags_n>0) strcat(tag_clause, " AND id IN(SELECT file FROM filestags WHERE tag");
				else strcpy(tag_clause, " id IN(SELECT file FROM filestags WHERE tag");;
				for(unsigned short i=0; i<search->or_tag_elements_n; i++){
					if(i>0) strcat(tag_clause, " INTERSECT SELECT file FROM filestags WHERE tag");
					strcat(tag_clause, " IN(");
					for(unsigned short j=0; j<search->or_tag_elements[i].or_number; j++){
						if(j>0) strcat(tag_clause, ", ");
						sprintf(where_clause, "%llu", search->or_tag_elements[i].ids[j]);
						strcat(tag_clause, where_clause);
					}
					strcat(tag_clause, ")");
				}
				if(search->include_tags_n==0 && search->exclude_tags_n>0) strcat(tag_clause, ")");
			}
			strcat(tag_clause, ")");
			add_where_clause(search->sql, tag_clause);
		}
	}
	switch(search->order_by){
		case none:
			break;
		case size:
			strcat(search->sql, " ORDER BY size");
			break;
		case random_order:
			strcat(search->sql, " ORDER BY RANDOM()");
			break;
	}
	if(search->order_by!=none && search->descending) strcat(search->sql, " DESC");
	if(search->limit!=0){
		char limit_str[30];
		sprintf(limit_str, " LIMIT %lu", search->limit);
		strcat(search->sql, limit_str);
	}
	strcat(search->sql, ";");
	if(strlen(search->sql)>(SEARCH_MAX_SQL_LEN-2)) return -1;	//too long
	else return 0;
}

void free_search(struct search* search){
	if(search->output_ids.size>0) free(search->output_ids.data);
}

