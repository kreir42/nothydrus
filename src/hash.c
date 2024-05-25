#include <xxhash.h>

#include "nothydrus.h"
#include "hash.h"

void xxhash_file(void* hash_ptr, char* filepath){
	FILE* fp = fopen(filepath, "rb");

	//calculates hash of file, copied from xxhash documentation
	XXH3_state_t* state = XXH3_createState();
	XXH3_128bits_reset(state);
	char buffer[4096];
	size_t count;
	while ((count = fread(buffer, 1, sizeof(buffer), fp)) != 0) {
		XXH3_128bits_update(state, buffer, count);
	}
	*(XXH128_hash_t*)hash_ptr = XXH3_128bits_digest(state);
	XXH3_freeState(state);

	fclose(fp);
}
