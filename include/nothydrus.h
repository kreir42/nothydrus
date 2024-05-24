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

void init();

void add_files(char** paths, unsigned int paths_n, int_least8_t add_flags);
#define ADD_STDIN	(1<<0)

#endif
