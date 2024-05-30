#include "nothydrus.h"

struct id_dynarr new_id_dynarr(unsigned long initial_size){
	struct id_dynarr dynarr = {
		.used = 0,
		.size = initial_size>0 ? initial_size : 1,
		.data = NULL
	};
	dynarr.data = malloc(dynarr.size * sizeof(sqlite3_int64));
	return dynarr;
}

static inline void grow_dynarr(struct id_dynarr* dynarr, unsigned long size){
	dynarr->size += size;
	dynarr->data = realloc(dynarr->data, dynarr->size * sizeof(sqlite3_int64));
}

void append_id_dynarr(struct id_dynarr* dynarr, sqlite3_int64 id){
	if(dynarr->size == dynarr->used){
		grow_dynarr(dynarr, 20 + dynarr->size*0.2);
	}
	dynarr->data[dynarr->used] = id;
	dynarr->used++;
}

void crop_id_dynarr(struct id_dynarr* dynarr){
	if(dynarr->used <= MIN_ID_DYNARR_SIZE){
		dynarr->data = realloc(dynarr->data, MIN_ID_DYNARR_SIZE * sizeof(sqlite3_int64));
		dynarr->size = MIN_ID_DYNARR_SIZE;
		return;
	}
	dynarr->data = realloc(dynarr->data, dynarr->used * sizeof(sqlite3_int64));
	dynarr->size = dynarr->used;
}
