#include "nothydrus.h"

void add_files(char** paths, unsigned int paths_n, int_least8_t add_flags){
	for(unsigned int i=0; i<paths_n; i++){
		puts(paths[i]);
	}
	if(add_flags & ADD_FILES_STDIN){
		puts("Also adding paths from stdin");
		size_t linesize = 48;
		char* line = malloc(linesize*sizeof(char));
		unsigned int counter = 0;
		while(getline(&line, &linesize, stdin)!=-1){
			line[strlen(line)-1] = '\0';	//remove newline
			puts(line);
			counter++;
		}
		printf("Added %d paths from stdin\n", counter);
		free(line);
	}
}
