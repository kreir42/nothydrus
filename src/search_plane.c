#include "nothydrus.h"
#include "tui.h"

struct ncplane* search_plane(struct search* search_to_copy){
	struct search* search = malloc(sizeof(struct search));
	if(search_to_copy==NULL){
		search->sql[0] = '\0';
		search->input_ids_n = 0;
		search->input_ids = NULL;
		search->output_ids = new_id_dynarr(MIN_ID_DYNARR_SIZE);
	}else{
		strcpy(search->sql, search_to_copy->sql);
		search->input_ids_n = search_to_copy->input_ids_n;
		if(search->input_ids_n > 0){
			search->input_ids = malloc(search->input_ids_n*sizeof(sqlite3_int64));
			memcpy(search->input_ids, search_to_copy->input_ids, search->input_ids_n*sizeof(sqlite3_int64));
		}else search->input_ids = NULL;
		search->output_ids = new_id_dynarr(search_to_copy->output_ids.used);
		memcpy(search->output_ids.data, search_to_copy->output_ids.data, search->output_ids.used);
	}

	unsigned int screen_rows, screen_cols;
	notcurses_stddim_yx(nc, &screen_rows, &screen_cols);
	struct ncplane_options plane_options = {
		.y = 0, .x = 0,
		.rows = screen_rows, .cols = screen_cols,
		.userptr = search
	};
	struct ncplane* plane = ncpile_create(nc, &plane_options);

	strcpy(search->sql, "SELECT id FROM files;");
	run_search(search);
	//TBD display

	return plane;
}

void free_search_plane(struct ncplane* plane){
	struct search* search = ncplane_userptr(plane);
	if(search->input_ids!=NULL) free(search->input_ids);
	if(search->output_ids.size>0) free(search->output_ids.data);
	free(search);
	ncplane_destroy(plane);
}
