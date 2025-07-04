#include <dirent.h>
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

static void add_directory(char* dir_path, int_least8_t flags, unsigned int* added, unsigned int* failed){
	DIR* dir = opendir(dir_path);
	if(dir == NULL){
		perror("Error opening directory");
		(*failed)++;
		return;
	}

	struct dirent* entry;
	while((entry = readdir(dir)) != NULL){
		if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
		char* new_path = malloc(strlen(dir_path) + strlen(entry->d_name) + 2);
		sprintf(new_path, "%s/%s", dir_path, entry->d_name);

		struct stat st;
		if(lstat(new_path, &st)){
			perror("Error in add_directory lstat");
			(*failed)++;
			free(new_path);
			continue;
		}

		if(S_ISDIR(st.st_mode)){
			add_directory(new_path, flags, added, failed);
		}else if(S_ISREG(st.st_mode)){
			if(add_file(new_path, flags)) (*failed)++;
			else (*added)++;
		}else if(S_ISLNK(st.st_mode)){
			struct stat link_st;
			if(stat(new_path, &link_st)){
				perror("Error statting symlink target");
				(*failed)++;
			}else{
				if(S_ISDIR(link_st.st_mode)){
					if(flags & ADD_FILES_FOLLOW_DIR_SYMLINKS){
						add_directory(new_path, flags, added, failed);
					}
				}else if(S_ISREG(link_st.st_mode)){
					if(add_file(new_path, flags)) (*failed)++;
					else (*added)++;
				}
			}
		}
		free(new_path);
	}
	closedir(dir);
}

void add_files(char** paths, unsigned int paths_n, int_least8_t flags){
	unsigned int added=0, failed=0;
	for(unsigned int i=0; i<paths_n; i++){
		char* path = transform_input_path(paths[i]);
		if(path==NULL){failed++; continue;}

		struct stat st;
		if(stat(path, &st)){
			perror("Error in add_files stat");
			failed++;
			free(path);
			continue;
		}
		
		if(S_ISDIR(st.st_mode)){
			if(flags & ADD_FILES_RECURSIVE){
				add_directory(path, flags, &added, &failed);
			}else{
				fprintf(stderr, "Error: %s is a directory, but --recursive flag not specified\n", path);
				failed++;
			}
		}else if(S_ISREG(st.st_mode)){
			if(add_file(path, flags)) failed++;
			else added++;
		}else if(S_ISLNK(st.st_mode)){
			struct stat link_st;
			if(stat(path, &link_st)){
				perror("Error statting symlink target");
				failed++;
			}else{
				if(S_ISDIR(link_st.st_mode)){
					if(flags & ADD_FILES_FOLLOW_DIR_SYMLINKS){
						add_directory(path, flags, &added, &failed);
					}
				}else if(S_ISREG(link_st.st_mode)){
					if(add_file(path, flags)) failed++;
					else added++;
				}
			}
		}
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

			struct stat st;
			if(stat(path, &st)){
				perror("Error in add_files stat");
				failed++;
				free(path);
				continue;
			}

			if(S_ISDIR(st.st_mode)){
				if(flags & ADD_FILES_RECURSIVE){
					add_directory(path, flags, &added, &failed);
				} else {
					fprintf(stderr, "Error: %s is a directory, but --recursive flag not specified\n", path);
					failed++;
				}
			} else if(S_ISREG(st.st_mode)){
				if(add_file(path, flags)) failed++;
				else added++;
			}
			counter++;
			free(path);
		}
		printf("Added %d paths from stdin\n", counter);
		free(line);
	}
	printf("Call to add_files finished: %u succeeded, %u failed\n", added, failed);
}
