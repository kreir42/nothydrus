#include "nothydrus.h"
#include "tui.h"

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
					fprintf(stderr, "Error: add requires one or more paths, passed as arguments and/or piped through stdin\n");
					end_program();
					return -1;
				}
				add_files(NULL, 0, add_flags);
			}else{
				add_files(&argv[i], argc-i, add_flags);
			}
			end_program();
			break;
		}else if(!strcmp(argv[i], "sql-search")){
			struct search search;
			search.input_ids = NULL;
			search.output_ids = new_id_dynarr(MIN_ID_DYNARR_SIZE);
			//TBD! error checking
			i++;
			strcpy(search.sql, argv[i]);
			puts(search.sql);
			start_program(0);	//TBD! flag for search
			run_search(&search);
			for(unsigned long i=0; i<search.output_ids.used; i++){
				printf("%lld\n", search.output_ids.data[i]);
			}
			end_program();
			break;
		}else{
			fprintf(stderr, "Error: unrecognized argument %s\n", argv[i]);
			return -1;
		}
	}
	if(argc==1){	//TBD change to allow config using parameters
		start_program(START_PROGRAM_TUI);
		start_tui(0);
		end_program();
	}
	return 0;
}
