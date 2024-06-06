#include "nothydrus.h"
#include "hash.h"

short check_file(sqlite3_int64 id, int_least8_t flags){
	char* filepath = filepath_from_id(id);
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

void check_files(void* data, int_least8_t flags){
	unsigned int failed=0, passed=0;
	if(data==NULL){
		if(!(flags&CHECK_FILES_STDIN)){
			//TBD check all files
			return;
		}
	}else{
		if(flags&CHECK_FILES_INPUT_PATHS){	//input NULL-terminated list of paths
			char** paths = data;
			unsigned int i=0;
			sqlite3_int64 id;
			while(paths[i]!=NULL){
				i++;
				id = id_from_filepath(paths[i]);
				if(id!=0 && check_file(id, flags)) failed++;
				else passed++;
			}
		}else{
			struct id_dynarr* id_dynarr;
			if(flags&CHECK_FILES_INPUT_SEARCH){
				struct search* search = data;
				id_dynarr = &search->output_ids;
			}else{
				id_dynarr = data;
			}
			for(unsigned int i=0; i<id_dynarr->used; i++){
				if(check_file(id_dynarr->data[i], flags)) failed++;
				else passed++;
			}
		}
	}
	if(flags&CHECK_FILES_STDIN){
		if(flags&CHECK_FILES_INPUT_PATHS){
			size_t linesize = 64;
			char* line = malloc(linesize*sizeof(char));
			sqlite3_int64 id;
			while(getline(&line, &linesize, stdin)!=-1){
				id = id_from_filepath(line);
				if(id!=0 && check_file(id, flags)) failed++;
				else passed++;
			}
			free(line);
		}else{
			size_t linesize = 12;
			char* line = malloc(linesize*sizeof(char));
			while(getline(&line, &linesize, stdin)!=-1){
				if(check_file(strtoll(line, NULL, 10), flags)) failed++;
				else passed++;
			}
			free(line);
		}
	}
	printf("Call to check_files finished: %u passed, %u failed\n", passed, failed);
}
