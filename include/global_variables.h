#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include <sqlite3.h>

#include "constants.h"

extern sqlite3* main_db;

extern sqlite3_stmt* add_file_statement;
extern sqlite3_stmt* filepath_from_id_statement;
extern sqlite3_stmt* flags_from_id_statement;
extern sqlite3_stmt* filesize_from_id_statement;
extern sqlite3_stmt* hash_from_id_statement;
extern sqlite3_stmt* id_from_filepath_statement;
extern sqlite3_stmt* set_file_flags_statement;
extern sqlite3_stmt* add_flag_to_file_statement;
extern sqlite3_stmt* remove_flag_from_file_statement;
extern sqlite3_stmt* add_tag_statement;
extern sqlite3_stmt* tag_id_from_name_statement;
extern sqlite3_stmt* tag_name_from_id_statement;
extern sqlite3_stmt* tag_statement;
extern sqlite3_stmt* untag_statement;
extern sqlite3_stmt* search_tags_statement;
extern sqlite3_stmt* get_file_tags_statement;
extern sqlite3_stmt* add_custom_column_statement;
extern sqlite3_stmt* get_file_columns_statement;
extern sqlite3_stmt* search_file_from_hash_statement;
extern sqlite3_stmt* find_file_from_hash_and_size_statement;
extern sqlite3_stmt* find_file_from_hash_and_size_with_flags_statement;
extern sqlite3_stmt* set_filepath_statement;

extern unsigned short custom_columns_n;
extern struct custom_column* custom_columns;

extern char main_path[MAX_PATH];	//path where .nothydrus directory is located
extern char execution_path[MAX_PATH];

#endif
