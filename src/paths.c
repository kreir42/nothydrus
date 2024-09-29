#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "paths.h"
#include "global_variables.h"
#include "constants.h"

char main_path[MAX_PATH];	//path where .nothydrus directory is located
char execution_path[MAX_PATH];

short set_main_path(){
	if(getcwd(execution_path, MAX_PATH*sizeof(char))==NULL){
		perror("Error in getcwd(execution_path)");
		return 1;
	}
	strcpy(main_path, execution_path);
	while(strlen(main_path)>1){
		char check_path[MAX_PATH];
		snprintf(check_path, sizeof(char)*MAX_PATH, "%s/"INIT_DIRECTORY, main_path);
		if(access(check_path, F_OK)==0){
			return 0;
		}else{
			char* last_slash = strrchr(main_path, '/');
			*last_slash = '\0';
		}
	}
	return 1;
}

char* transform_input_path(char* input_path){
	//turn input path into an absolute path
	char result[MAX_PATH];
	if(input_path[0]=='/'){	//path is absolute, but might have links/etc
		realpath(input_path, result);
	}else{
		char buff[MAX_PATH];
		snprintf(buff, sizeof(char)*MAX_PATH, "%s/%s", execution_path, input_path);
		realpath(buff, result);
	}
	//check if the input path is under the main path
	if(strncmp(result, main_path, strlen(main_path))!=0){
		fprintf(stderr, "Error in transform_input_path: input_path '%s' is not under the main path\n", input_path);
		return NULL;
	}
	//convert to relative path and return a new string
	return strdup(result + strlen(main_path)+1);
}

char* transform_output_path(char* path){
	char* result = realpath(path, NULL);
	return result;
}
