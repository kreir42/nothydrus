#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include <sqlite3.h>

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
extern sqlite3_stmt* add_taggroup_statement;
extern sqlite3_stmt* tag_id_from_name_statement;
extern sqlite3_stmt* taggroup_id_from_name_statement;
extern sqlite3_stmt* tag_name_from_id_statement;
extern sqlite3_stmt* tag_fullname_from_id_statement;
extern sqlite3_stmt* taggroup_name_from_id_statement;
extern sqlite3_stmt* tag_statement;
extern sqlite3_stmt* untag_statement;
extern sqlite3_stmt* search_tags_statement;
extern sqlite3_stmt* get_file_tags_statement;

#endif
