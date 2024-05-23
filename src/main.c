#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "constants.h"

int main(int argc, char** argv){
	char* target_dir = ".";
	int error_code = 0;
	for(unsigned short i=1; i<argc; i++){
		if(!strcmp(argv[i], "init")){
			i++;
			if(i==(argc-1)){
				target_dir = argv[i];
			}else{
				fprintf(stderr, "Error: init takes only one argument\n");
				return -1;
			}
			error_code = chdir(target_dir);
			if(error_code){
				fprintf(stderr, "Error %d: could not chdir to directory %s. ", error_code, target_dir);
				perror(NULL);
				break;
			}
			error_code = mkdir(INIT_DIRECTORY, DEFAULT_DIRECTORY_MODE);
			if(error_code){
				perror("Error when trying to create init directory");
				break;
			}
		}else{
			fprintf(stderr, "Error: unrecognized argument %s\n", argv[i]);
			return -1;
		}
	}
	return error_code;
}
