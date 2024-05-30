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
			i++;
			short output_id = 0;
			if(!strcmp(argv[i], "--id")){
				output_id = 1;
				i++;
			}else if(!strcmp(argv[i], "--filepath")) i++;
			if(i!=argc-1){
				fprintf(stderr, "Error: sql-search command requires one argument\n");
				return -1;
			}
			strcpy(search.sql, argv[i]);
			start_program(START_PROGRAM_SQL_SEARCH);
			run_search(&search);
			if(output_id){
				for(unsigned long i=0; i<search.output_ids.used; i++){
					printf("%lld\n", search.output_ids.data[i]);
				}
			}else{
				for(unsigned long i=0; i<search.output_ids.used; i++){
					printf("%s\n", filepath_from_id(search.output_ids.data[i]));
				}
			}
			free_search(&search);
			end_program();
			break;
		}else if(!strcmp(argv[i], "display")){
			i++;
			struct search search;
			search.input_ids_n = 0;
			search.input_ids = NULL;
			search.output_ids = new_id_dynarr(10);
			for(unsigned short j=i; j<argc; j++){
				append_id_dynarr(&search.output_ids, strtoll(argv[j], NULL, 10));
				fprintf(stderr, "added %d/%d value %lld\n", j, argc, strtoll(argv[j], NULL, 10));
			}
			fprintf(stderr, "--------%ld/%ld--------\n", search.output_ids.used, search.output_ids.size);
			for(unsigned short j=0; j<search.output_ids.used; j++){
				fprintf(stderr, "%lld\n", search.output_ids.data[j]);
			}
			if(search.output_ids.used==0){
				fprintf(stderr, "Error: display command requires at least one argument\n");
				free_search(&search);
				return -1;
			}
			start_program(START_PROGRAM_DISPLAY);
			start_tui(START_TUI_DISPLAY, &search);
			free_search(&search);
			end_program();
			break;
		}else{
			fprintf(stderr, "Error: unrecognized argument %s\n", argv[i]);
			return -1;
		}
	}
	if(argc==1){	//TBD change to allow config using parameters
		start_program(START_PROGRAM_TUI);
		start_tui(0, NULL);
		end_program();
	}
	return 0;
}
