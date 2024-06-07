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


struct id_dynarr{
	unsigned long used;
	unsigned long size;
	sqlite3_int64* data;
};
struct id_dynarr new_id_dynarr(unsigned long initial_size);
void append_id_dynarr(struct id_dynarr* dynarr, sqlite3_int64 id);
void crop_id_dynarr(struct id_dynarr* dynarr);
struct search{
	char sql[SEARCH_MAX_SQL_LEN];
	unsigned int input_ids_n;
	sqlite3_int64* input_ids;	//NULL for all
	struct id_dynarr output_ids;
};
short run_search(struct search* search);
void free_search(struct search* search);

void start_program(int_least8_t flags);
#define START_PROGRAM_INIT		(1<<0)
#define START_PROGRAM_ADD_FILES		(1<<1)
#define START_PROGRAM_TUI		(1<<2)
#define START_PROGRAM_SQL_SEARCH	(1<<3)
#define START_PROGRAM_DISPLAY		(1<<4)

void end_program();


//user functions

void init();

void add_files(char** paths, unsigned int paths_n, int_least8_t flags);
#define ADD_FILES_STDIN	(1<<0)
short check_file(sqlite3_int64 id, int_least8_t flags);
void check_files(void* data, int_least8_t flags);//TBD check all files if no input
#define CHECK_FILES_INPUT_IDS		(1<<0)
#define CHECK_FILES_INPUT_SEARCH	(1<<1)
#define CHECK_FILES_STDIN		(1<<2)
#define CHECK_FILES_HASH		(1<<3)

void add_tag(char* name, char* taggroup_name);//TBD
void tag(sqlite3_int64 file_id, sqlite3_int64 tag_id);//TBD
void untag(sqlite3_int64 file_id, sqlite3_int64 tag_id);//TBD
void add_taggroup(char* taggroup_name);//TBD


//util functions
char* filepath_from_id(sqlite3_int64 id);
int flags_from_id(sqlite3_int64 id);
sqlite3_int64 filesize_from_id(sqlite3_int64 id);
char* hash_from_id(sqlite3_int64 id);
sqlite3_int64 id_from_filepath(char* filepath);
void set_file_flags(sqlite3_int64 id, int_least8_t flags);
void add_flag_to_file(sqlite3_int64 id, int_least8_t flag);
void remove_flag_from_file(sqlite3_int64 id, int_least8_t flag);

sqlite3_int64 tag_id_from_name(char* name, sqlite3_int64 taggroup);
sqlite3_int64 taggroup_id_from_name(char* name);
char* tag_name_from_id(sqlite3_int64 id, sqlite3_int64* taggroup);

#endif
