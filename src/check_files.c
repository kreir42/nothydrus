#include "nothydrus.h"
#include "hash.h"

short check_file(sqlite3_int64 id, int_least8_t flags){
	char* filepath = filepath_from_id(id);
	if(!filepath){fprintf(stderr, "Error: no filepath found for id %lld\n", id); return -1;}
	printf("Checking file %s\n", filepath);
	struct stat st;
	if(stat(filepath, &st)){
		perror("Error in check_file stat:");
		goto check_failed;
	}
	if(!S_ISREG(st.st_mode)){
		fprintf(stderr, "Error: is not a regular file\n");
		goto check_failed;
	}
	if(filesize_from_id(id)!=st.st_size){
		fprintf(stderr, "Error: failed filesize check\n");
		goto check_failed;
	}
	if(flags&CHECK_FILES_HASH){
		char hash[HASH_SIZE];
		xxhash_file(&hash, filepath);
		char* db_hash = hash_from_id(id);
		for(unsigned short i=0; i<HASH_SIZE; i++){
			if(hash[i]!=db_hash[i]) goto check_failed;
		}
	}
	remove_flag_from_file(id, FILE_MISSING);
	return 0;

	check_failed:
	add_flag_to_file(id, FILE_MISSING);
	return -1;
}

short check_filepath(char* filepath, int_least8_t flags){
	printf("Checking filepath %s\n", filepath);
	struct stat st;
	if(stat(filepath, &st)){
		perror("Error in check_file stat:");
		return -1;
	}
	if(!S_ISREG(st.st_mode)){
		fprintf(stderr, "Error: is not a regular file\n");
		return -1;
	}
	char hash[HASH_SIZE];
	xxhash_file(&hash, filepath);
	struct id_dynarr matching_files = new_id_dynarr(MIN_ID_DYNARR_SIZE);
	search_file_from_hash(hash, st.st_size, &matching_files);
	if(matching_files.used==1){
		check_file(matching_files.data[0], flags);
		if(flags_from_id(matching_files.data[0])&FILE_MISSING){
			remove_flag_from_file(matching_files.data[0], FILE_MISSING);
			free(matching_files.data);
			return 1;
		}else{
			free(matching_files.data);
			return 0;
		}
	}else if(matching_files.used>1){
		fprintf(stderr, "%ld matching files found in database\n", matching_files.used);
		return -1;
	}else{
		fprintf(stderr, "No matching file found in database\n");
		return -1;
	}
}

//stdin is a list of paths, unless CHECK_FILES_INPUT_IDS, in which case its a list of ids
void check_files(int_least8_t flags, int argc, char** argv){
	unsigned int failed=0, passed=0, found_in_db=0, not_found_in_db=0, found_and_replaced=0;
	short check_filepaths=0;
	if(flags&CHECK_FILES_HASH && !(flags&CHECK_FILES_IN_DATABASE)) check_filepaths=1;

	if(argc==0 && !(flags&CHECK_FILES_STDIN)){
		struct search search = {.order_by=none, .limit=0, .min_size=0, .max_size=0};
		search.output_ids = new_id_dynarr(MIN_ID_DYNARR_SIZE);
		compose_search_sql(&search);
		run_search(&search);
		for(unsigned int i=0; i<search.output_ids.used; i++){
			if(check_file(search.output_ids.data[i], flags)) failed++;
			else passed++;
		}
		free(search.output_ids.data);
	}else{
		unsigned short r=0;
		struct id_dynarr id_dynarr = new_id_dynarr(MIN_ID_DYNARR_SIZE);
		if(flags&CHECK_FILES_INPUT_IDS){ //ids in arguments
			while(r<argc){
				append_id_dynarr(&id_dynarr, strtoll(argv[r], NULL, 10));
				r++;
			}
		}else{ //paths in arguments
			sqlite3_int64 id;
			while(r<argc){
				char* path = transform_input_path(argv[r]);
				if(path!=NULL){
					id = id_from_filepath(path);
					if(id==-1){
						fprintf(stderr, "Filepath %s not found in database\n", path);
						if(check_filepaths){
							short result = check_filepath(path, flags);
							if(result<0) not_found_in_db++;
							else{
								found_in_db++;
								if(result>0) found_and_replaced++;
							}
						}
					}else append_id_dynarr(&id_dynarr, id);
					free(path);
				}
				r++;
			}
		}
		for(unsigned int i=0; i<id_dynarr.used; i++){
			if(check_file(id_dynarr.data[i], flags)) failed++;
			else passed++;
		}
		free(id_dynarr.data);
	}

	if(flags&CHECK_FILES_STDIN){
		if(flags&CHECK_FILES_INPUT_IDS){
			size_t linesize = 12;
			char* line = malloc(linesize*sizeof(char));
			while(getline(&line, &linesize, stdin)!=-1){
				if(check_file(strtoll(line, NULL, 10), flags)) failed++;
				else passed++;
			}
			free(line);
		}else{
			size_t linesize = 64;
			char* line = malloc(linesize*sizeof(char));
			sqlite3_int64 id;
			while(getline(&line, &linesize, stdin)!=-1){
				line[strlen(line)-1] = '\0';	//remove newline
				char* path = transform_input_path(line);
				if(path==NULL){
					failed++;
					continue;
				}
				id = id_from_filepath(path);
				if(id==-1){
					fprintf(stderr, "Filepath %s not found in database\n", line);
					if(check_filepaths){
						short result = check_filepath(path, flags);
						if(result<0) not_found_in_db++;
						else{
							found_in_db++;
							if(result>0) found_and_replaced++;
						}
					}
				}else if(check_file(id, flags)) failed++;
				else passed++;
				free(path);
			}
			free(line);
		}
	}
	printf("Call to check_files finished: %u passed, %u failed\n", passed, failed);
	printf("   Filepaths not in database: %u found, %u replaced, %u not found\n", found_in_db, found_and_replaced, not_found_in_db);
}
