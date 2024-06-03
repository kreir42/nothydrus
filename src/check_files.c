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
		unsigned char hash[16];
		xxhash_file(&hash, filepath);
	}
	return 0;

	check_failed:
	return -1;
}

void check_files(void* data, int_least8_t flags){
	if(data==NULL){
		if(!(flags&CHECK_FILES_STDIN)){
		}
	}else{
		if(flags&CHECK_FILES_INPUT_PATHS){	//input NULL-terminated list of paths
			char* paths = data;
		}else if(flags&CHECK_FILES_INPUT_SEARCH){
			struct search* search = data;
		}else{
			struct id_dynarr* id_dynarr = data;
		}
	}
	if(flags&CHECK_FILES_STDIN){
	}
}
