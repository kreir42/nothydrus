#include <xxhash.h>

#include "nothydrus.h"
#include "hash.h"

uint64_t xxhash_file(char* filepath){
	FILE* fp = fopen(filepath, "rb");

	//calculates hash of file, copied from xxhash documentation
	XXH3_state_t* state = XXH3_createState();
	XXH3_64bits_reset(state);
	char buffer[4096];
	size_t count;
	while ((count = fread(buffer, 1, sizeof(buffer), fp)) != 0) {
		XXH3_64bits_update(state, buffer, count);
	}
	uint64_t hash = XXH3_64bits_digest(state);
	XXH3_freeState(state);

	fclose(fp);
	return hash;
}
