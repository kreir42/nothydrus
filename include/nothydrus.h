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

struct search{
	char sql[SEARCH_MAX_SQL_LEN];
	unsigned int input_ids_n;
	sqlite3_int64* input_ids;	//NULL for all
	unsigned int output_ids_n;
	sqlite3_int64* output_ids;	//NULL for none
};
short run_search(struct search* search);

void start_program(int_least8_t flags);
#define START_PROGRAM_INIT	(1<<0)
#define START_PROGRAM_ADD_FILES	(1<<1)
#define START_PROGRAM_TUI	(1<<2)

void end_program();


//user functions

void init();

void add_files(char** paths, unsigned int paths_n, int_least8_t flags);
#define ADD_FILES_STDIN	(1<<0)

//util functions
char* filepath_from_id(sqlite3_int64 id);	//must free() afterwards

#endif
