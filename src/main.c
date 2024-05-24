#include "nothydrus.h"

sqlite3* main_db;

int main(int argc, char** argv){
	char* target_dir = ".";
	for(unsigned short i=1; i<argc; i++){
		if(!strcmp(argv[i], "init")){
			i++;
			if(i==(argc-1)){
				target_dir = argv[i];
			}else if(i!=argc){
				fprintf(stderr, "Error: init takes only one argument\n");
				return -1;
			}
			if(chdir(target_dir)){
				fprintf(stderr, "Error: could not chdir to directory %s. ", target_dir);
				perror(NULL);
				break;
			}
			init();
			break;
		}else if(!strcmp(argv[i], "add")){
			start_program(START_PROGRAM_ADD_FILES);
			i++;
			int_least8_t add_flags = 0;
			if(!isatty(fileno(stdin))) add_flags |= ADD_FILES_STDIN;
			if(i==argc){
				if(!add_flags&ADD_FILES_STDIN){
					fprintf(stderr, "Error: add requires one or more paths, passed as arguments or piped through stdin\n");
					end_program();
					return -1;
				}
				add_files(NULL, 0, add_flags);
			}else{
				add_files(&argv[i], argc-i, add_flags);
			}
			end_program();
			break;
		}else{
			fprintf(stderr, "Error: unrecognized argument %s\n", argv[i]);
			return -1;
		}
	}
	return 0;
}
