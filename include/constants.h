#ifndef CONSTANTS_H
#define CONSTANTS_H


#define VERSION "0.1"
#define PROGRAM_NAME "nothydrus"
#define INIT_DIRECTORY "."PROGRAM_NAME
#define MAIN_DATABASE_NAME "main.db"
#define DEFAULT_TAGGROUP_NAME "default"

#define DEFAULT_DIRECTORY_MODE S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH

#define NON_CUSTOM_FILE_COLUMNS 6
#define MIN_UI_ELEMENTS 5
#define SEARCH_MAX_SQL_LEN 4000
#define MIN_ID_DYNARR_SIZE 100
#define HASH_SIZE 16
#define WHERE_CLAUSE_SIZE 200
#define TAG_CLAUSE_SIZE 2000
#define TAG_SEARCH_ROWS 30
#define TAG_SEARCH_COLS 50
#define CUSTOM_COLUMN_NAME_SIZE 100
#include <limits.h>
#define MAX_PATH PATH_MAX

//filetypes
#define FILETYPE_NONE 0
#define FILETYPE_OTHER 1
#define FILETYPE_IMAGE 2
#define FILETYPE_VIDEO 3

//file flags
#define FILE_MISSING	1

//custom columns
#define COLUMN_NOT_NULL 1
#define COLUMN_NO_LOWER_LIMIT 2
#define COLUMN_NO_UPPER_LIMIT 4

//custom column types
#define COLUMN_TYPE_TEXT 0
#define COLUMN_TYPE_INTEGER 1
#define COLUMN_TYPE_REAL 2

#endif
