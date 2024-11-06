#include "nothydrus.h"
#include "tui.h"

sqlite3* main_db;

int main(int argc, char** argv){
	for(unsigned short i=1; i<argc; i++){
		if(!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")){
			puts(PROGRAM_NAME" v"VERSION);
			puts("Usage: "PROGRAM_NAME" [command] [command option(s)] [argument(s)]");
			puts("Commands:");
			puts("     init [target directory]");
			puts("          Creates a database in \"target directory\"/"INIT_DIRECTORY", or in "INIT_DIRECTORY" at the current path if a target directory is not given.");
			puts("     add [target file path(s)]");
			puts("          Adds the target files to the database, if not already there.");
			puts("          Paths can be piped through stdin.");
			puts("     sql-search [--filepath/--id] [sql search]");
			puts("          Returns the result of an SQL query. The query must return a single integer column.");
			puts("          By default, the result is assumed to be file ids, and converted to filepaths. This can be specified with the --filepath command option.");
			puts("          --id option will skip the conversion to filepath and return the number directly.");
			puts("     display [file id(s)]");
			puts("          Given a series of file ids as arguments and/or piped in through stdin, displays them fullscreen in a TUI.");
			puts("     check [options] [file(s)]");
			puts("          Checks the presence of files in the database, given as arguments or piped in through stdin.");
			puts("          --id option will make the command expect file ids instead of filepaths.");
			puts("          --hash option will make the command also check the hash of the file. Otherwise, it only checks the file exists and has the correct size.");
//			puts("          --in-database option");
//			puts("          --missing option");
			puts("          If given no files, will check all files in the database.");
			puts("     add_tag [tag] [taggroup]");
			puts("          Adds a single new tag to the database. Can provide a taggroup as a second argument, otherwise the default taggroup is assumed.");
			puts("     add_taggroup [taggroup]");
			puts("          Adds a single new taggroup to the database.");	//TBD multiple taggroups in single command
			puts("     tag [--add] [tag] [--taggroup taggroup] [file path(s)]");
			puts("          Tags files whose filepaths are given as arguments or piped in with tag.");
			puts("          --add option will add the tag and taggroup if they don't already exist.");
			puts("          --taggroup option after the tag allows you to specify the tag's taggroup. Otherwise, the default taggroup is assumed.");
			puts("     add_custom_column [name] [type] [flags] [lower limit] [upper limit]");
			puts("          Arguments must be (in order): name, type (\"text\", \"integer\" or \"real\"), flags (1 for NOT NULL), lower limit, upper limit");	//TBD? special value MAX for limits
			return 0;
		}else if(!strcmp(argv[i], "init")){
			i++;
			char* target_dir = ".";
			if(i==(argc-1)){
				target_dir = argv[i];
			}else if(i!=argc){
				fprintf(stderr, "Error: init takes only one argument\n");
				return -1;
			}
			if(chdir(target_dir)){
				fprintf(stderr, "Error: could not chdir to directory %s", target_dir);
				perror(NULL);
				return -1;
			}
			init();
			break;
		}else if(!strcmp(argv[i], "add")){
			if(set_main_path()){ fprintf(stderr, "Error: could not locate main path\n"); return 1;}
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
			if(set_main_path()){ fprintf(stderr, "Error: could not locate main path\n"); return 1;}
			start_program(START_PROGRAM_SQL_SEARCH);
			run_search(&search);
			if(output_id){
				for(unsigned long i=0; i<search.output_ids.used; i++){
					printf("%lld\n", search.output_ids.data[i]);
				}
			}else{
				for(unsigned long i=0; i<search.output_ids.used; i++){
					char* path = transform_output_path(filepath_from_id(search.output_ids.data[i]));
					printf("%s\n", path);
					free(path);
				}
			}
			free_search(&search);
			end_program();
			break;
		}else if(!strcmp(argv[i], "display")){
			i++;
			struct search search;
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
			if(set_main_path()){ fprintf(stderr, "Error: could not locate main path\n"); return 1;}
			start_program(0);
			start_tui(START_TUI_DISPLAY, &search);
			free_search(&search);
			end_program();
			break;
		}else if(!strcmp(argv[i], "check")){
			int_least8_t flags = 0;
			i++;
			if(!isatty(fileno(stdin))){
				flags |= CHECK_FILES_STDIN;
			}
			if(set_main_path()){fprintf(stderr, "Error: could not locate main path\n"); return 1;}
			start_program(0);
			if(i<argc){
				while(argv[i][0]=='-' && argv[i][1]=='-'){
					if(!strcmp(argv[i], "--id")) flags |= CHECK_FILES_INPUT_IDS;
					else if(!strcmp(argv[i], "--hash")) flags |= CHECK_FILES_HASH;
					else if(!strcmp(argv[i], "--in-database")) flags |= CHECK_FILES_IN_DATABASE;
					else if(!strcmp(argv[i], "--missing")) flags |= CHECK_FILES_MISSING;
					else{
						fprintf(stderr, "Error: unrecognized check option %s\n", argv[i]);
						return -1;
					}
					i++;
				}
			}
			check_files(flags, argc-i, &argv[i]);
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
			if(set_main_path()){ fprintf(stderr, "Error: could not locate main path\n"); return 1;}
			start_program(0);
			sqlite3_int64 taggroup_id;
			if(i+1<argc) taggroup_id = taggroup_id_from_name(argv[i+1]);
			else taggroup_id = 1;
			add_tag(argv[i], taggroup_id);
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
			if(set_main_path()){ fprintf(stderr, "Error: could not locate main path\n"); return 1;}
			start_program(0);
			add_taggroup(argv[i]);
			end_program();
			return 0;
		}else if(!strcmp(argv[i], "tag")){
			i++;
			char add_flag = 0;
			if(!strcmp(argv[i], "--add")){
				add_flag = 1;
				i++;
			}
			if(argc-i<2){
				fprintf(stderr, "Error: tag command requires at least two arguments\n");
				return -1;
			}
			char* tag_name = argv[i];
			if(set_main_path()){ fprintf(stderr, "Error: could not locate main path\n"); return 1;}
			start_program(START_PROGRAM_TAG);
			sqlite3_int64 tag_id, taggroup_id;
			if(i+1<argc && !strcmp(argv[i+1], "--taggroup")){
				taggroup_id = taggroup_id_from_name(argv[i+2]);
				if(taggroup_id==-1){
					if(add_flag){
						printf("taggroup '%s' not found, adding\n", argv[i+2]);
						add_taggroup(argv[i+2]);
						taggroup_id = taggroup_id_from_name(argv[i+2]);
					}else{
						fprintf(stderr, "Error: taggroup not found in database\n");
						return -1;
					}
				}
				i+=3;
			}else{
				taggroup_id = 1;
				i++;
			}
			tag_id = tag_id_from_name(tag_name, taggroup_id);
			if(tag_id==-1){
				if(add_flag){
					printf("tag '%s' not found, adding\n", tag_name);
					add_tag(tag_name, taggroup_id);
					tag_id = tag_id_from_name(tag_name, taggroup_id);
				}else{
					fprintf(stderr, "Error: tag not found in database\n");
					return -1;
				}
			}
			sqlite3_int64 file_id;
			while(i<argc){
				char* path = transform_input_path(argv[i]);
				if(path!=NULL){
					file_id = id_from_filepath(path);
					free(path);
					if(file_id==-1){
						fprintf(stderr, "Error: file not found in database\n");
					}else{
						tag(file_id, tag_id);
					}
				}
				i++;
			}
			if(!isatty(fileno(stdin))){
				size_t linesize = 48;
				char* line = malloc(linesize*sizeof(char));
				while(getline(&line, &linesize, stdin)!=-1){
					line[strlen(line)-1] = '\0';	//remove newline
					char* path = transform_input_path(line);
					if(path!=NULL){
						file_id = id_from_filepath(path);
						free(path);
						if(file_id==-1){
							fprintf(stderr, "Error: file not found in database\n");
							continue;
						}
						tag(file_id, tag_id);
					}
				}
				free(line);
			};
			end_program();
			return 0;
		}else if(!strcmp(argv[i], "add_custom_column")){
			i++;
			if(argc-i!=5){
				fprintf(stderr, "Error: wrong number of arguments for add_custom_column\n");
				return -1;
			}
			short new_custom_column_type;
			if(!strcmp(argv[i+1], "text")){
				new_custom_column_type = COLUMN_TYPE_TEXT;
			}else if(!strcmp(argv[i+1], "integer")){
				new_custom_column_type = COLUMN_TYPE_INTEGER;
			}else if(!strcmp(argv[i+1], "real")){
				new_custom_column_type = COLUMN_TYPE_REAL;
			}else{
				fprintf(stderr, "Error: unrecognized \"type\" argument %s for add_custom_column. See --help\n", argv[i+1]);
				return -1;
			}
			if(set_main_path()){ fprintf(stderr, "Error: could not locate main path\n"); return 1;}
			start_program(0);
			add_custom_column(argv[i], new_custom_column_type, strtol(argv[i+2], NULL, 10), strtol(argv[i+3], NULL, 10), strtol(argv[i+4], NULL, 10));
			end_program();
			return 0;
		}else{
			fprintf(stderr, "Error: unrecognized argument %s\n", argv[i]);
			fprintf(stderr, "Run "PROGRAM_NAME" -h for help\n");
			return -1;
		}
	}
	if(argc==1){	//TBD change to allow config using parameters
		if(set_main_path()){ fprintf(stderr, "Error: could not locate main path\n"); return 1;}
		start_program(START_PROGRAM_TUI);
		start_tui(0, NULL);
		end_program();
	}
	return 0;
}
