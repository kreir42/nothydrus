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

	enum {byte, kilobyte, megabyte, gigabyte} min_size_unit=2, max_size_unit=2;
	unsigned int min_size=search->min_size/1000000, max_size=search->max_size/1000000;
	char min_size_unit_str[3], max_size_unit_str[3];
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
			case NCKEY_ENTER:
			case ' ':
				if(ui_index>=tag_elements){
					switch(ui_index-tag_elements){
						case 0:	//order by
							char* order_options[] = {"None", "Size", "Random", NULL};
							search->order_by = chooser(current_plane, order_options, search->order_by);
							if(search->order_by==none || search->order_by==random_order){
								search->descending = 0;
							}else{
								char* descending_options[] = {"Ascending", "Descending", NULL};
								search->descending = chooser(current_plane, descending_options, search->descending);
							}
							search_not_run = 1;
							break;
						case 1:	//limit
							struct ncreader_options ncreader_options = {.flags=NCREADER_OPTION_CURSOR};
							struct ncplane_options reader_plane_options = {
								.y = 2+ui_index, .x = 23,
								.rows = 1, .cols = 20,
							};
							struct ncplane* reader_subplane = ncplane_create(current_plane, &reader_plane_options);
							struct ncreader* reader = ncreader_create(reader_subplane, &ncreader_options);
							struct ncplane* reader_plane = ncreader_plane(reader);
							struct ncinput reader_input;
							ncplane_erase_region(current_plane, 2+ui_index, 23, 1, 20);
							do{
								ncpile_render(reader_plane);
								ncpile_rasterize(reader_plane);
								c=notcurses_get(nc, NULL, &reader_input);
								ncreader_offer_input(reader, &reader_input);
							}while(c!=NCKEY_ENTER);
							char* reader_contents;
							ncreader_destroy(reader, &reader_contents);
							search->limit = strtol(reader_contents, NULL, 10);
							free(reader_contents);
							//ncplane_destroy(reader_subplane); //TBD why doesnt this work???
							break;
						case 2:	//min_size
							break;
						case 3:	//max_size
							break;
					}
				}else{
				}
				break;
			case '+':
				if(ui_index>=tag_elements) switch(ui_index-tag_elements){
					case 1:	//limit
						search->limit++;
						break;
					case 2:	//min_size
						min_size++;
						break;
					case 3:	//max_size
						max_size++;
						break;
				}
				break;
			case '-':
				if(ui_index>=tag_elements) switch(ui_index-tag_elements){
					case 1:	//limit
						if(search->limit>0) search->limit--;
						break;
					case 2:	//min_size
						if(min_size>0) min_size--;
						break;
					case 3:	//max_size
						if(max_size>0) max_size--;
						break;
				}
				break;
		}
		//update min, max sizes
		switch(min_size_unit){
			case byte:
				search->min_size = min_size;
				min_size_unit_str[0] = '\0';
				break;
			case kilobyte:
				search->min_size = min_size*1000;
				strcpy(min_size_unit_str, "KB");
				break;
			case megabyte:
				search->min_size = min_size*1000000;
				strcpy(min_size_unit_str, "MB");
				break;
			case gigabyte:
				search->min_size = min_size*1000000000;
				strcpy(min_size_unit_str, "GB");
				break;
		}
		switch(max_size_unit){
			case byte:
				search->max_size = max_size;
				max_size_unit_str[0] = '\0';
				break;
			case kilobyte:
				search->max_size = max_size*1000;
				strcpy(max_size_unit_str, "KB");
				break;
			case megabyte:
				search->max_size = max_size*1000000;
				strcpy(max_size_unit_str, "MB");
				break;
			case gigabyte:
				search->max_size = max_size*1000000000;
				strcpy(max_size_unit_str, "GB");
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
		if(search->order_by!=none && search->order_by!=random_order){
			if(search->descending) ncplane_putstr(current_plane, " descending");
			else ncplane_putstr(current_plane, " ascending");
		}
		//limit
		ncplane_printf_yx(current_plane, 1+tag_elements+2, 3, "Limit (0 for none): %lu", search->limit);
		//min, max size
		ncplane_printf_yx(current_plane, 1+tag_elements+3, 3, "Min size: %u %s", min_size, min_size_unit_str);
		ncplane_printf_yx(current_plane, 1+tag_elements+4, 3, "Max size (0 for none): %u %s", max_size, max_size_unit_str);
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
