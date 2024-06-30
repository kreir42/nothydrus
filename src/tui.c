#include "nothydrus.h"
#include "tui.h"

struct notcurses* nc;

static struct search_plane{
	struct ncplane* plane;
	struct search* search;
}* search_planes;

char search_not_run = 1;

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

char* input_reader(struct ncplane* parent_plane, int y, int x, int h, int w){
	struct ncreader_options ncreader_options = {.flags=NCREADER_OPTION_CURSOR};
	struct ncplane_options reader_plane_options = {
		.y = y, .x = x,
		.rows = h, .cols = w,
	};
	struct ncplane* reader_subplane = ncplane_create(parent_plane, &reader_plane_options);
	struct ncreader* reader = ncreader_create(reader_subplane, &ncreader_options);
	struct ncplane* reader_plane = ncreader_plane(reader);
	struct ncinput reader_input;
	ncplane_erase_region(parent_plane, y, x, h, w);	//TBD? get initial value from here?
	do{
		ncpile_render(reader_plane);
		ncpile_rasterize(reader_plane);
		if(notcurses_get(nc, NULL, &reader_input)==NCKEY_ENTER) break;
		ncreader_offer_input(reader, &reader_input);
	}while(1);
	char* reader_contents;
	ncreader_destroy(reader, &reader_contents);
	//ncplane_destroy(reader_subplane); //TBD why doesnt this work???
	return reader_contents;
}

static void search_tags(struct id_dynarr* dynarr, char* tag_search){
	char* tag_name = tag_search;
	char* taggroup_name = "%";	//TBD implement taggroup search
	for(unsigned short i=strlen(tag_name)-1; i>0; i--){
		if(tag_name[i]==' ') tag_name[i]='\0';	//remove trailing whitespace
		else break;
	}

	//TBD put strlen in variable
	char* tag_name_search = malloc((strlen(tag_name)+3)*sizeof(char));
	if(tag_name[0]=='"' && tag_name[strlen(tag_name)-1]=='"'){
		tag_name++;
		sprintf(tag_name_search, "%s", tag_name);
		tag_name_search[strlen(tag_name_search)-1] = '\0';
	}else{
		sprintf(tag_name_search, "%%%s%%", tag_name);
	}
	char* taggroup_name_search = malloc((strlen(taggroup_name)+3)*sizeof(char));
	if(taggroup_name[0]=='"' && taggroup_name[strlen(taggroup_name)-1]=='"'){
		taggroup_name++;
		sprintf(taggroup_name_search, "%s", taggroup_name);
		taggroup_name_search[strlen(taggroup_name_search)-1] = '\0';
	}else{
		sprintf(taggroup_name_search, "%%%s%%", taggroup_name);
	}

	dynarr->used = 0;
	sqlite3_clear_bindings(search_tags_statement);
	if(sqlite3_reset(search_tags_statement)!=SQLITE_OK){
		fprintf(stderr, "sqlite3_reset(search_tags_statement) returned an error: %s\n", sqlite3_errmsg(main_db));
	}
	sqlite3_bind_text(search_tags_statement, 1, taggroup_name_search, -1, SQLITE_STATIC);
	sqlite3_bind_text(search_tags_statement, 2, tag_name_search, -1, SQLITE_STATIC);
	int error_code;
	while((error_code=sqlite3_step(search_tags_statement)) == SQLITE_ROW){
		append_id_dynarr(dynarr, sqlite3_column_int64(search_tags_statement, 0));
	}
	if(error_code != SQLITE_DONE){
		fprintf(stderr, "Error executing search_tags_statement at row %ld: %s\n", dynarr->used, sqlite3_errmsg(main_db));
	}
	free(tag_name_search);
	free(taggroup_name_search);
}

static void add_tag_to_search(char exclude_flag, sqlite3_int64 tag_id, struct search* search){
	//TBD search for duplicate tags
	if(exclude_flag){
		search->exclude_tags_n++;
		search->exclude_tags = realloc(search->exclude_tags, sizeof(sqlite3_int64)*search->exclude_tags_n);
		search->exclude_tags[search->exclude_tags_n-1] = tag_id;
	}else{
		search->include_tags_n++;
		search->include_tags = realloc(search->include_tags, sizeof(sqlite3_int64)*search->include_tags_n);
		search->include_tags[search->include_tags_n-1] = tag_id;
	}
	search_not_run = 1;
}

static void add_tag_tui(struct ncplane* parent_plane, struct search* search){
	struct ncplane_options plane_options = {
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.rows = TAG_SEARCH_ROWS+2, .cols = TAG_SEARCH_COLS,	//TBD take max size into account
		.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED
	};
	struct ncplane* plane = ncplane_create(parent_plane, &plane_options);
	plane_options.x = NCALIGN_RIGHT;
	struct ncplane* or_plane = ncplane_create(parent_plane, &plane_options);

	char* tag_search, *tag_search_ptr;
	struct id_dynarr search_results = new_id_dynarr(10);
	char exclude_flag=0, or_plane_flag=0;
	sqlite3_int64* or_tags = NULL;
	unsigned short or_tags_n = 0;
	unsigned short ui_index=0, ui_elements=1, or_ui_index=0;
	uint32_t c = NCKEY_RESIZE;
	do{
		switch(c){
			case NCKEY_DOWN:
				if(or_plane_flag){
					if(or_ui_index<or_tags_n-1) or_ui_index++;
					else or_ui_index = 0;
				}else{
					if(ui_index<ui_elements-1) ui_index++;
					else ui_index = 0;
				}
				break;
			case NCKEY_UP:
				if(or_plane_flag){
					if(or_ui_index>0) or_ui_index--;
					else or_ui_index = or_tags_n-1;
				}else{
					if(ui_index>0) ui_index--;
					else ui_index = ui_elements-1;
				}
				break;
			case NCKEY_RIGHT:
			case NCKEY_LEFT:
				if(or_tags_n>0){
					if(or_plane_flag) or_plane_flag=0;
					else or_plane_flag=1;
				}
				break;
			case 'g':
				ui_index = 0;
				break;
			case NCKEY_ENTER:
				if(or_plane_flag){
				}else{
					if(ui_index==0){
						tag_search = input_reader(plane, 0, 2, 1, TAG_SEARCH_COLS);
						tag_search_ptr = tag_search;
						while(*tag_search_ptr==' ') tag_search_ptr++;	//skip begginning whitespace
						if(tag_search_ptr[0]=='-'){
							tag_search_ptr++;
							exclude_flag = 1;
						}else exclude_flag = 0;
						search_tags(&search_results, tag_search_ptr);
						ncplane_putstr_yx(plane, 0, 2, tag_search);
						free(tag_search);
					}else{
						add_tag_to_search(exclude_flag, search_results.data[ui_index-1], search);
						goto end_label;
					}
				}
				break;
			case ' ':
				if(or_plane_flag){
				}else{
					if(ui_index>0){
						//TBD check for doubles
						or_tags_n++;
						or_tags = realloc(or_tags, sizeof(sqlite3_int64)*or_tags_n);
						or_tags[or_tags_n-1] = search_results.data[ui_index-1];
					}
				}
				break;
			case 'a':
				if(or_tags_n>0){
					if(or_tags_n==1){
						add_tag_to_search(exclude_flag, or_tags[0], search);
					}else{
						search->or_tag_elements_n++;
						search->or_tag_elements = realloc(search->or_tag_elements, sizeof(struct or_tag_element)*search->or_tag_elements_n);
						search->or_tag_elements[search->or_tag_elements_n-1].or_number = or_tags_n;
						search->or_tag_elements[search->or_tag_elements_n-1].ids = malloc(or_tags_n*sizeof(sqlite3_int64));
						memcpy(search->or_tag_elements[search->or_tag_elements_n-1].ids, or_tags, or_tags_n*sizeof(sqlite3_int64));
						search_not_run=1;
					}
					goto end_label;
				}
				break;
			case 'A':	//TBD change to OR
				if(search_results.used>0){
					for(unsigned short i=0; i<search_results.used; i++){
						add_tag_to_search(exclude_flag, search_results.data[i], search);
					}
					goto end_label;
				}
				break;
		}
		ncplane_erase(plane);
		ncplane_erase(or_plane);
		ui_elements = 1 + search_results.used;
		if(ui_elements>1+TAG_SEARCH_ROWS) ui_elements = 1+TAG_SEARCH_ROWS;
		//mark cursor position
		if(or_plane_flag){
			ncplane_putstr_yx(or_plane, 1+or_ui_index, 0, "->");
		}else{
			if(ui_index==0) ncplane_putstr_yx(plane, 0, 0, "->");
			else ncplane_putstr_yx(plane, 1+ui_index, 0, "->");
		}
		//print search results
		for(unsigned short i=0; i<search_results.used && i<TAG_SEARCH_ROWS; i++){
			ncplane_putstr_yx(plane, i+2, 2, tag_fullname_from_id(search_results.data[i]));
		}
		ncpile_render(plane);
		ncpile_rasterize(plane);
		//OR plane
		if(or_tags_n>0){
			//TBD box
			ncplane_putstr_yx(or_plane, 0, 2, "Any of:");
			for(unsigned short i=0; i<or_tags_n; i++){
				ncplane_putstr_yx(or_plane, 1+i, 2, tag_fullname_from_id(or_tags[i]));
			}
		}
		ncpile_render(or_plane);
		ncpile_rasterize(or_plane);
		c = notcurses_get(nc, NULL, NULL);
	}while(c!='q' && c!='Q');
	end_label:
	ncplane_destroy(plane);
	ncplane_destroy(or_plane);
	if(search_results.data!=NULL) free(search_results.data);
	if(or_tags!=NULL) free(or_tags);
}

static unsigned short size_unit_from_ptr(char* ptr, unsigned short init_value){
	while(*ptr==' ') ptr++;	//skip whitespace
	if(*ptr=='B') return 0;
	if(*ptr=='K') return 1;
	if(*ptr=='M') return 2;
	if(*ptr=='G') return 3;
	return init_value;
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
	unsigned long long min_size=search->min_size/1000000, max_size=search->max_size/1000000;
	char min_size_unit_str[3], max_size_unit_str[3];
	char* reader_result, * size_unit_ptr;
	unsigned short ui_index = 0, tag_elements = 0;
	unsigned short ui_elements = MIN_UI_ELEMENTS + tag_elements;
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
			case 'g':
				ui_index = 0;
				break;
			case 'G':
				ui_index = ui_elements;
				break;
			case 't':
				add_tag_tui(current_plane, search);
				break;
			case NCKEY_ENTER:
			case ' ':
				switch(ui_index){
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
						reader_result = input_reader(current_plane, 1+ui_index, 23, 1, 12);
						search->limit = strtol(reader_result, NULL, 10);
						free(reader_result);
						search_not_run = 1;
						break;
					case 2:	//min_size
						reader_result = input_reader(current_plane, 1+ui_index, 13, 1, 18);
						min_size = strtoull(reader_result, &size_unit_ptr, 10);
						min_size_unit = size_unit_from_ptr(size_unit_ptr, min_size_unit);
						free(reader_result);
						search_not_run = 1;
						break;
					case 3:	//max_size
						reader_result = input_reader(current_plane, 1+ui_index, 26, 1, 18);
						max_size = strtoull(reader_result, &size_unit_ptr, 10);
						max_size_unit = size_unit_from_ptr(size_unit_ptr, max_size_unit);
						free(reader_result);
						search_not_run = 1;
						break;
					case 4:	//add tag
						add_tag_tui(current_plane, search);
						break;
					default:
						break;
				}
				break;
			case '+':
				switch(ui_index){
					case 1:	//limit
						search->limit++;
						search_not_run = 1;
						break;
					case 2:	//min_size
						min_size++;
						search_not_run = 1;
						break;
					case 3:	//max_size
						max_size++;
						search_not_run = 1;
						break;
				}
				break;
			case '-':
				switch(ui_index){
					case 1:	//limit
						if(search->limit>0){
							search->limit--;
							search_not_run = 1;
						}
						break;
					case 2:	//min_size
						if(min_size>0){
							min_size--;
							search_not_run = 1;
						}
						break;
					case 3:	//max_size
						if(max_size>0){
							max_size--;
							search_not_run = 1;
						}
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
		tag_elements = search->include_tags_n + search->exclude_tags_n + search->or_tag_elements_n;
		ui_elements = MIN_UI_ELEMENTS + tag_elements;
		compose_search_sql(search);
		ncplane_erase(current_plane);
		//mark current index
		if(ui_index<MIN_UI_ELEMENTS-1) ncplane_putstr_yx(current_plane, 1+ui_index, 0, "->");
		else ncplane_putstr_yx(current_plane, 2+ui_index, 0, "->");
		//number of results
		ncplane_printf_yx(current_plane, 0, 0, "Results: %ld", search_planes[current_search_plane_i].search->output_ids.used);
		if(search_not_run){
			ncplane_putstr(current_plane, " (search not run)");
			ncplane_cursor_move_rel(current_plane, 0, -strlen("(search not run)"));
			ncplane_format(current_plane, 0, -1, 1, 0, NCSTYLE_ITALIC);
		}
		//order by
		ncplane_putstr_yx(current_plane, 1, 3, "Order by: ");
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
		ncplane_printf_yx(current_plane, 2, 3, "Limit (0 for none): %lu", search->limit);
		//min, max size
		ncplane_printf_yx(current_plane, 3, 3, "Min size: %llu %s", min_size, min_size_unit_str);
		ncplane_printf_yx(current_plane, 4, 3, "Max size (0 for none): %llu %s", max_size, max_size_unit_str);
		//add new tag button
		ncplane_putstr_yx(current_plane, 6, 3, "Add new tag");
		//tags
		for(unsigned short i=0; i<search->include_tags_n; i++){
			ncplane_putstr_yx(current_plane, 2+MIN_UI_ELEMENTS+i, 3, tag_fullname_from_id(search->include_tags[i]));
		}
		for(unsigned short i=0; i<search->exclude_tags_n; i++){
			ncplane_printf_yx(current_plane, 2+MIN_UI_ELEMENTS+search->include_tags_n+i, 3, "-%s", tag_fullname_from_id(search->exclude_tags[i]));
		}
		for(unsigned short i=0; i<search->or_tag_elements_n; i++){
			//TBD
		}
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
