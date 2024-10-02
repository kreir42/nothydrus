#include "hash.h"

#include "nothydrus.h"

static struct {
	char* extension;
	short filetype;
} extensions_table[] = {
	"png", FILETYPE_IMAGE,
	"jpg", FILETYPE_IMAGE,
	"jpeg", FILETYPE_IMAGE,
	"webp", FILETYPE_IMAGE,
	"mkv", FILETYPE_VIDEO,
	"mp4", FILETYPE_VIDEO,
	"avi", FILETYPE_VIDEO,
	"webm", FILETYPE_VIDEO,
	"mpg", FILETYPE_VIDEO,
	"mpeg", FILETYPE_VIDEO,
	"wmv", FILETYPE_VIDEO,
	"mov", FILETYPE_VIDEO,
	"gif", FILETYPE_VIDEO,
	NULL, FILETYPE_NONE
};

static short get_filetype(char* filepath){
	char* extension = strrchr(filepath, '.');
	if(extension==NULL){
		return FILETYPE_NONE;
	}else extension++;
	unsigned short i=0;
	while(extensions_table[i].extension!=NULL){
		if(!strcasecmp(extension, extensions_table[i].extension)) return extensions_table[i].filetype;
		else i++;
	}
	return FILETYPE_OTHER;
}

static short add_file(char* filepath, int_least8_t flags){
	printf("Adding file: %s\n", filepath);
	struct stat st;
	if(stat(filepath, &st)){
		perror("Error in add_file stat:");
		return -1;
	}
	if(!S_ISREG(st.st_mode)){
		fprintf(stderr, "Error: is not a regular file\n");
		return -1;
	}
	//hash
	unsigned char hash[HASH_SIZE];
	xxhash_file(&hash, filepath);
	sqlite3_bind_blob(add_file_statement, 1, hash, HASH_SIZE, SQLITE_STATIC);
	//filesize
	sqlite3_bind_int64(add_file_statement, 2, st.st_size);
	//filetype
	short filetype = get_filetype(filepath);
	if(filetype == FILETYPE_NONE || filetype == FILETYPE_OTHER){
		fprintf(stderr, "Error: has unrecognized or no extension\n");
		return -1;
	}
	sqlite3_bind_int64(add_file_statement, 3, filetype);
	//flags
	sqlite3_bind_int(add_file_statement, 4, flags);
	//filepath
	sqlite3_bind_text(add_file_statement, 5, filepath, -1, SQLITE_STATIC);

	if(sqlite3_step(add_file_statement) != SQLITE_DONE){
		fprintf(stderr, "sqlite3_step(add_file_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		sqlite3_clear_bindings(add_file_statement);
		sqlite3_reset(add_file_statement);
		return -1;
	}
	sqlite3_clear_bindings(add_file_statement);
	if(sqlite3_reset(add_file_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(add_file_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
		return -1;
	}
	return 0;
}

void add_files(char** paths, unsigned int paths_n, int_least8_t flags){
	unsigned int added=0, failed=0;
	for(unsigned int i=0; i<paths_n; i++){
		char* path = transform_input_path(paths[i]);
		if(path==NULL){failed++; continue;}
		else if(add_file(path, flags)) failed++;
		else added++;
		free(path);
	}
	if(flags & ADD_FILES_STDIN){
		puts("Adding paths from stdin:");
		size_t linesize = 48;
		char* line = malloc(linesize*sizeof(char));
		unsigned int counter = 0;
		while(getline(&line, &linesize, stdin)!=-1){
			line[strlen(line)-1] = '\0';	//remove newline
			char* path = transform_input_path(line);
			if(path==NULL){failed++; continue;}
			else if(add_file(path, flags)) failed++;
			else added++;
			counter++;
			free(path);
		}
		printf("Added %d paths from stdin\n", counter);
		free(line);
	}
	printf("Call to add_files finished: %u succeeded, %u failed\n", added, failed);
}
