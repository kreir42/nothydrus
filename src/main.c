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
			}
			if(!isatty(fileno(stdin))){
				size_t linesize = 12;
				char* line = malloc(linesize*sizeof(char));
				while(getline(&line, &linesize, stdin)!=-1){
					append_id_dynarr(&search.output_ids, strtoll(line, NULL, 10));
				}
				free(line);
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
		}else if(!strcmp(argv[i], "check")){
			int_least8_t flags = 0;
			i++;
			if(argc==i){
				if(isatty(fileno(stdin))){
					fprintf(stderr, "Error: display command requires at least one argument\n");
					return -1;
				}
			}else{
				while(argv[i][0]=='-' && argv[i][1]=='-'){
					if(!strcmp(argv[i], "--id")) flags |= CHECK_FILES_INPUT_IDS;
					else if(!strcmp(argv[i], "--hash")) flags |= CHECK_FILES_HASH;
					else{
						fprintf(stderr, "Error: unrecognized check option %s\n", argv[i]);
						return -1;
					}
					i++;
				}
			}
			if(!isatty(fileno(stdin))){
				flags |= CHECK_FILES_STDIN;
			}
			start_program(0);
			if(i<argc){
				struct id_dynarr id_dynarr = new_id_dynarr(10);
				if(flags&CHECK_FILES_INPUT_IDS){ //ids in arguments
					while(i<argc){
						append_id_dynarr(&id_dynarr, strtoll(argv[i], NULL, 10));
						i++;
					}
				}else{ //paths in arguments
					sqlite3_int64 id;
					while(i<argc){
						id = id_from_filepath(argv[i]);
						if(id!=-1) append_id_dynarr(&id_dynarr, id);
						i++;
					}
				}
				check_files(&id_dynarr, flags);
				free(id_dynarr.data);
			}else{
				check_files(NULL, flags);
			}
			end_program();
			return 0;
		}else if(!strcmp(argv[i], "add_tag")){
			i++;
			if(i==argc){
				fprintf(stderr, "Error: add_tag command requires at least one argument\n");
				return -1;
			}else if(i+2<argc){
				fprintf(stderr, "Error: add_tag command has too many arguments\n");
				return -1;
			}
			start_program(0);
			char* taggroup_name;
			if(i+1<argc) taggroup_name = argv[i+1];
			else taggroup_name = NULL;
			add_tag(argv[i], taggroup_name);
			end_program();
			return 0;
		}else if(!strcmp(argv[i], "add_taggroup")){
			i++;
			if(i==argc){
				fprintf(stderr, "Error: add_taggroup command requires one argument\n");
				return -1;
			}else if(i+1<argc){
				fprintf(stderr, "Error: too many arguments for taggroup command\n");
				return -1;
			}
			start_program(0);
			add_taggroup(argv[i]);
			end_program();
			return 0;
		}else if(!strcmp(argv[i], "tag")){
			i++;
			if(argc-i<2){
				fprintf(stderr, "Error: add_tag command requires at least two arguments\n");
				return -1;
			}
			char* tag_name = argv[i];
			start_program(0);
			sqlite3_int64 tag_id, taggroup_id;
			if(!strcmp(argv[i+1], "--taggroup")){
				taggroup_id = taggroup_id_from_name(argv[i+2]);
				if(taggroup_id==-1){
					fprintf(stderr, "Error: taggroup not found in database\n");
					return -1;
				}
				i+=3;
			}else{
				taggroup_id = 1;
				i++;
			}
			tag_id = tag_id_from_name(tag_name, taggroup_id);
			if(tag_id==-1){
				fprintf(stderr, "Error: tag not found in database\n");
				return -1;
			}
			sqlite3_int64 file_id;
			while(i<argc){
				file_id = id_from_filepath(argv[i]);
				i++;
				if(file_id==-1){
					fprintf(stderr, "Error: file not found in database\n");
					continue;
				}
				tag(file_id, tag_id);
			}
			end_program();
			return 0;
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
