#include "nothydrus.h"
#include "tui.h"

struct ncplane* new_search_plane(struct search* search_to_copy){
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

	strcpy(search->sql, "SELECT id FROM files ORDER BY RANDOM();");
	run_search(search);

	return plane;
}

void search_plane(struct ncplane* plane){
	struct search* search = ncplane_userptr(plane);
	unsigned long i = 0;
	ncplane_printf(plane, "Results: 1/%ld", search->output_ids.used);
	display_file(search->output_ids.data[0], 0, plane);
	ncpile_render(plane);
	ncpile_rasterize(plane);
	uint32_t c;
	while((c=notcurses_get(nc, NULL, NULL))!='q'){
		switch(c){
			case NCKEY_RIGHT:
				i++;
				if(i>=search->output_ids.used-1) i=0;
				break;
			case NCKEY_LEFT:
				if(i==0) i = search->output_ids.used-1;
				else i--;
				break;
		}
		reset_display_plane(plane);
		ncplane_printf_yx(plane, 0, 0, "Results: %ld/%ld", i+1, search->output_ids.used);
		display_file(search->output_ids.data[i], 0, plane);
		ncpile_render(plane);
		ncpile_rasterize(plane);
	}
}

void free_search_plane(struct ncplane* plane){
	struct search* search = ncplane_userptr(plane);
	if(search->input_ids!=NULL) free(search->input_ids);
	if(search->output_ids.size>0) free(search->output_ids.data);
	free(search);
	ncplane_destroy(plane);
}
