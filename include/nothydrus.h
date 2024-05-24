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



void start_program(int_least8_t flags);
#define START_PROGRAM_INIT	(1<<0)
#define START_PROGRAM_ADD_FILES	(1<<1)

void end_program();



//user functions

void init();

void add_files(char** paths, unsigned int paths_n, int_least8_t flags);
#define ADD_FILES_STDIN	(1<<0)

#endif
