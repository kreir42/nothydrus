#ifndef NOTHYDRUS_H
#define NOTHYDRUS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sqlite3.h>

#include "constants.h"
#include "global_variables.h"
#include "paths.h"


struct id_dynarr{
	unsigned long used;
	unsigned long size;
	sqlite3_int64* data;
};
struct id_dynarr new_id_dynarr(unsigned long initial_size);
void append_id_dynarr(struct id_dynarr* dynarr, sqlite3_int64 id);
void crop_id_dynarr(struct id_dynarr* dynarr);

struct custom_column{
	char name[CUSTOM_COLUMN_NAME_SIZE];
	short type;
	uint_least8_t flags;
	int lower_limit;
	int upper_limit;
};
void prepare_add_custom_column();
void get_custom_columns();

struct or_tag_element{
	unsigned short or_number;	//number of ids pointed to by ids
	sqlite3_int64* ids;
};
enum order_by_enum {none, size, random_order};
struct search{
	char sql[SEARCH_MAX_SQL_LEN];
	enum order_by_enum order_by;	//TBD order by other, custom numbers(number of tags in expression, rating field...)
	char descending;
	unsigned long limit;
	unsigned long min_size;
	unsigned long max_size;
	uint_least8_t filetypes;
	char* filepath;
	unsigned short include_tags_n;
	sqlite3_int64* include_tags;
	unsigned short exclude_tags_n;
	sqlite3_int64* exclude_tags;
	unsigned short or_tag_elements_n;
	struct or_tag_element* or_tag_elements;
	//TBD file flags
	//TBD number of tags matching expression
	struct id_dynarr output_ids;
};
short run_search(struct search* search);
short compose_search_sql(struct search* search);
void free_search(struct search* search);

void start_program(int_least8_t flags);	//TBD? enum instead of flag
#define START_PROGRAM_INIT		(1<<0)
#define START_PROGRAM_ADD_FILES		(1<<1)
#define START_PROGRAM_TUI		(1<<2)
#define START_PROGRAM_SQL_SEARCH	(1<<3)
#define START_PROGRAM_TAG		(1<<4)
void end_program();


//user functions
void init();
void add_files(char** paths, unsigned int paths_n, int_least8_t flags);
#define ADD_FILES_STDIN	(1<<0)
short check_file(sqlite3_int64 id, int_least8_t flags);
short check_filepath(char* filepath, int_least8_t flags);
void check_files(int_least8_t flags, int argc, char** argv);
#define CHECK_FILES_INPUT_IDS		(1<<0)
#define CHECK_FILES_INPUT_SEARCH	(1<<1)
#define CHECK_FILES_STDIN		(1<<2)
#define CHECK_FILES_HASH		(1<<3)
#define CHECK_FILES_IN_DATABASE		(1<<4)
#define CHECK_FILES_MISSING		(1<<5)
void add_tag(char* name);
void tag(sqlite3_int64 file_id, sqlite3_int64 tag_id);
void untag(sqlite3_int64 file_id, sqlite3_int64 tag_id);
void search_tags(struct id_dynarr* dynarr, char* tag_search);	//TBD? command
void add_custom_column(char* name, short type, uint_least8_t flags, int lower_limit, int upper_limit);
void set_custom_column_value(sqlite3_int64 file_id, unsigned short custom_column_id, void* value);

//util functions
char* filepath_from_id(sqlite3_int64 id);
int flags_from_id(sqlite3_int64 id);
sqlite3_int64 filesize_from_id(sqlite3_int64 id);
char* hash_from_id(sqlite3_int64 id);
sqlite3_int64 id_from_filepath(char* filepath);
void set_file_flags(sqlite3_int64 id, int_least8_t flags);
void add_flag_to_file(sqlite3_int64 id, int_least8_t flag);
void remove_flag_from_file(sqlite3_int64 id, int_least8_t flag);
void search_file_from_hash(char* hash, unsigned long filesize, struct id_dynarr* dynarr);
void set_filepath(sqlite3_int64 id, char* filepath);

sqlite3_int64 tag_id_from_name(char* name);
char* tag_name_from_id(sqlite3_int64 id);

void get_file_tags(sqlite3_int64 file_id, struct id_dynarr* dynarr);
void get_file_columns(sqlite3_int64 file_id);
#endif
