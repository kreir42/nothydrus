#include "hash.h"

#include "nothydrus.h"

static short add_file(char* filepath, int_least8_t flags){
	puts(filepath);	//TBD debug, remove
	uint64_t hash = xxhash_file(filepath);
	printf("%ld\n", hash);	//TBD debug, remove

	sqlite3_clear_bindings(add_file_statement);
	if(sqlite3_reset(add_file_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(add_file_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return -1;
	}
	return 0;
}

void add_files(char** paths, unsigned int paths_n, int_least8_t flags){
	for(unsigned int i=0; i<paths_n; i++){
		add_file(paths[i], flags);
	}
	if(flags & ADD_FILES_STDIN){
		puts("Adding paths from stdin:");
		size_t linesize = 48;
		char* line = malloc(linesize*sizeof(char));
		unsigned int counter = 0;
		while(getline(&line, &linesize, stdin)!=-1){
			line[strlen(line)-1] = '\0';	//remove newline
			add_file(line, flags);
			counter++;
		}
		printf("Added %d paths from stdin\n", counter);
		free(line);
	}
}
