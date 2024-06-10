#include "nothydrus.h"
#include "tui.h"

struct ncplane* new_search_plane(struct search* search_to_copy){
	struct search* search = malloc(sizeof(struct search));
	if(search_to_copy==NULL){
		search->sql[0] = '\0';
		search->output_ids = new_id_dynarr(MIN_ID_DYNARR_SIZE);
	}else{
		if(search_to_copy->sql[0]!='\0') strcpy(search->sql, search_to_copy->sql);
		search->output_ids = new_id_dynarr(search_to_copy->output_ids.used);
		memcpy(search->output_ids.data, search_to_copy->output_ids.data, search_to_copy->output_ids.used*sizeof(sqlite3_int64));
		search->output_ids.used = search_to_copy->output_ids.used;
	}

	unsigned int screen_rows, screen_cols;
	notcurses_stddim_yx(nc, &screen_rows, &screen_cols);
	struct ncplane_options plane_options = {
		.y = 0, .x = 0,
		.rows = screen_rows, .cols = screen_cols,
		.userptr = search
	};
	struct ncplane* plane = ncpile_create(nc, &plane_options);

	if(search_to_copy==NULL) strcpy(search->sql, "SELECT id FROM files ORDER BY RANDOM();");

	return plane;
}

void search_plane(struct ncplane* plane){
	struct search* search = ncplane_userptr(plane);
	if(search->sql[0]!='\0') run_search(search);
	unsigned int screen_rows, screen_cols;
	notcurses_stddim_yx(nc, &screen_rows, &screen_cols);	//TBD use search_plane instead of stdplane
	struct ncplane_options plane_options = {
		.y = 1, .x = 0,
		.rows = screen_rows-1, .cols = screen_cols,
	};
	struct ncplane* display_plane = ncplane_create(plane, &plane_options);
	unsigned long i = 0;
	ncplane_printf(plane, "Results: 1/%ld", search->output_ids.used);
	display_file(search->output_ids.data[0], 0, display_plane);
	ncpile_render(plane);
	ncpile_rasterize(plane);
	uint32_t c;
	while((c=notcurses_get(nc, NULL, NULL))!='q'){
		switch(c){
			case NCKEY_RIGHT:
				i++;
				if(i>=search->output_ids.used) i=0;
				reset_display_plane(display_plane);
				ncplane_erase_region(plane, 0, 0, 1, 0);
				ncplane_printf_yx(plane, 0, 0, "Results: %ld/%ld", i+1, search->output_ids.used);
				display_file(search->output_ids.data[i], 0, display_plane);
				break;
			case NCKEY_LEFT:
				if(i==0) i = search->output_ids.used-1;
				else i--;
				reset_display_plane(display_plane);
				ncplane_erase_region(plane, 0, 0, 1, 0);
				ncplane_printf_yx(plane, 0, 0, "Results: %ld/%ld", i+1, search->output_ids.used);
				display_file(search->output_ids.data[i], 0, display_plane);
				break;
			case 'e':
				display_file(search->output_ids.data[i], DISPLAY_FILE_EXTERNAL, display_plane);
				break;
			case 'm':
				display_file(search->output_ids.data[i], DISPLAY_FILE_MPV, display_plane);
				break;
		}
		ncpile_render(plane);
		ncpile_rasterize(plane);
	}
	reset_display_plane(display_plane);
}

void free_search_plane(struct ncplane* plane){
	struct search* search = ncplane_userptr(plane);
	free_search(search);
	free(search);
	ncplane_destroy(plane);
}
