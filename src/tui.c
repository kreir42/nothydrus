#include "nothydrus.h"
#include "tui.h"

struct notcurses* nc;

static struct search_plane{
	struct ncplane* plane;
	struct search* search;
}* search_planes;

static void new_search_plane(unsigned short i, struct search* search_to_copy){
	search_planes[i].search = malloc(sizeof(struct search));
	struct search* search = search_planes[i].search;
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
	search_planes[i].plane = ncpile_create(nc, &plane_options);

	if(search_to_copy==NULL) strcpy(search->sql, "SELECT id FROM files ORDER BY RANDOM();");
}

static inline void free_search_plane(unsigned short i){
	free_search(search_planes[i].search);
	free(search_planes[i].search);
	ncplane_destroy(search_planes[i].plane);
}

void start_tui(int_least8_t flags, void* data){
	if(!isatty(fileno(stdin))){
		freopen("/dev/tty", "r", stdin);	//reopen stdin if there was a pipe
	}

	struct notcurses_options opts = {
	};
	nc = notcurses_init(&opts, NULL);

	if(flags & START_TUI_DISPLAY){
		fullscreen_display((struct search*)data);
		notcurses_drop_planes(nc);
		notcurses_stop(nc);
		return;
	}
	unsigned short search_planes_n = 1, current_search_plane_i = 0;
	search_planes = malloc(search_planes_n*sizeof(struct search_plane));
	new_search_plane(0, NULL);
	struct ncplane* current_plane = search_planes[current_search_plane_i].plane;

	uint32_t c = 'r';
	do{
		switch(c){
			case 'q':
				goto end_label;	//TBD ask for confirmation
				break;
			case 'r':
				run_search(search_planes[current_search_plane_i].search);
				break;
			case 'f':
				if(search_planes[current_search_plane_i].search->output_ids.used>0) fullscreen_display(search_planes[current_search_plane_i].search);
				break;
		}
		current_plane = search_planes[current_search_plane_i].plane;
		ncplane_erase(current_plane);
		ncplane_printf_yx(current_plane, 2, 2, "Results: %ld", search_planes[current_search_plane_i].search->output_ids.used);
		ncpile_render(current_plane);
		ncpile_rasterize(current_plane);
	}while((c=notcurses_get(nc, NULL, NULL))!='Q');
	end_label:

	for(unsigned short i=0; i<search_planes_n; i++){
		free_search_plane(i);
	}
	free(search_planes);
	notcurses_drop_planes(nc);
	notcurses_stop(nc);
}
