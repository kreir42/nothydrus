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
	};
	search_planes[i].plane = ncpile_create(nc, &plane_options);
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

	struct timespec bugfix_timespec = {.tv_nsec = 100};
	while(notcurses_get(nc, &bugfix_timespec, NULL)){}	//bugfix for notcurses getting strange input on startup; TBD find cause of weird input

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
	struct search* search = search_planes[current_search_plane_i].search;

	unsigned short ui_index = 0, tag_elements = 0;
	unsigned short ui_elements = MIN_UI_ELEMENTS + tag_elements;

	char search_not_run = 1;
	uint32_t c = NCKEY_RESIZE;
	do{
		switch(c){
			case 'q':
				goto end_label;	//TBD ask for confirmation
				break;
			case 'r':
				run_search(search);
				search_not_run = 0;
				break;
			case 'f':
				if(search->output_ids.used>0) fullscreen_display(search);
				break;
			case NCKEY_DOWN:
				if(ui_index<ui_elements-1) ui_index++;
				else ui_index = 0;
				break;
			case NCKEY_UP:
				if(ui_index>0) ui_index--;
				else ui_index = ui_elements-1;
				break;
			case 'G':
				ui_index = ui_elements-1;
				break;
		}
		current_plane = search_planes[current_search_plane_i].plane;
		search = search_planes[current_search_plane_i].search;
		ui_elements = MIN_UI_ELEMENTS + tag_elements;
		compose_search_sql(search);
		ncplane_erase(current_plane);
		//mark current index
		if(ui_index>=tag_elements) ncplane_putstr_yx(current_plane, 2+ui_index, 0, "->");
		else ncplane_putstr_yx(current_plane, 1+ui_index, 0, "->");
		//number of results
		ncplane_printf_yx(current_plane, 0, 0, "Results: %ld", search_planes[current_search_plane_i].search->output_ids.used);
		if(search_not_run){
			ncplane_putstr(current_plane, " (search not run)");
			ncplane_cursor_move_rel(current_plane, 0, -strlen("(search not run)"));
			ncplane_format(current_plane, 0, -1, 1, 0, NCSTYLE_ITALIC);
		}
		//order by
		ncplane_putstr_yx(current_plane, 1+tag_elements+1, 3, "Order by: ");
		switch(search->order_by){
			case none:
				ncplane_putstr(current_plane, "none");
				break;
			case size:
				ncplane_putstr(current_plane, "size");
				break;
			case random_order:
				ncplane_putstr(current_plane, "random");
				break;
		}
		if(search->order_by!=none && search->order_by!=random){
			if(search->descending) ncplane_putstr(current_plane, " descending");
			else ncplane_putstr(current_plane, " ascending");
		}
		//limit
		ncplane_printf_yx(current_plane, 1+tag_elements+2, 3, "Limit (0 for none): %lu", search->limit);
		//min, max size
		ncplane_printf_yx(current_plane, 1+tag_elements+3, 3, "Min size: %lu", search->min_size);
		ncplane_printf_yx(current_plane, 1+tag_elements+4, 3, "Max size (0 for none): %lu", search->max_size);
		//SQL query
		ncplane_printf_yx(current_plane, ui_elements+3, 0, "SQL query: %s", search_planes[current_search_plane_i].search->sql);
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
