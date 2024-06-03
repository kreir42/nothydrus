#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include <sqlite3.h>

extern sqlite3* main_db;
extern sqlite3_stmt* add_file_statement;
extern sqlite3_stmt* filepath_from_id_statement;
extern sqlite3_stmt* flags_from_id_statement;
extern sqlite3_stmt* filesize_from_id_statement;

#endif
