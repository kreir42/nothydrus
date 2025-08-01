#include "nothydrus.h"
#include "tui.h"
#include <ctype.h>

struct notcurses* nc;
struct search* search;
struct ncplane* search_plane;
struct tui_options tui_options;

char search_not_run = 1;

static void new_search_plane(struct search* search_to_copy){
	search = malloc(sizeof(struct search));
	if(search_to_copy==NULL){
		search->sql[0] = '\0';
		search->output_ids = new_id_dynarr(MIN_ID_DYNARR_SIZE);
		search->order_by = tui_options.search_order_by;
		search->descending = tui_options.search_descending;
		search->limit = tui_options.search_limit;
		search->filetypes = 0;
		search->include_filepaths = NULL;
		search->include_filepaths_n = 0;
		search->exclude_filepaths = NULL;
		search->exclude_filepaths_n = 0;
		search->or_filepath_elements = NULL;
		search->or_filepath_elements_n = 0;
	}else{
		if(search_to_copy->sql[0]!='\0') strcpy(search->sql, search_to_copy->sql);
		search->output_ids = new_id_dynarr(search_to_copy->output_ids.used);
		memcpy(search->output_ids.data, search_to_copy->output_ids.data, search_to_copy->output_ids.used*sizeof(sqlite3_int64));
		search->output_ids.used = search_to_copy->output_ids.used;

		search->include_filepaths_n = search_to_copy->include_filepaths_n;
		if(search->include_filepaths_n > 0){
			search->include_filepaths = malloc(sizeof(char*) * search->include_filepaths_n);
			for(unsigned short i = 0; i < search->include_filepaths_n; i++){
				search->include_filepaths[i] = strdup(search_to_copy->include_filepaths[i]);
			}
		}
	}

	unsigned int screen_rows, screen_cols;
	notcurses_stddim_yx(nc, &screen_rows, &screen_cols);
	struct ncplane_options plane_options = {
		.y = 0, .x = 0,
		.rows = screen_rows, .cols = screen_cols,
	};
	search_plane = ncpile_create(nc, &plane_options);
}

char* input_reader(struct ncplane* parent_plane, int y, int x, int h, int w, const char* pre_text, const char* post_text, const char* prompt, bool box){
	struct ncplane_options plane_opts = {
		.y = y, .x = x,
		.rows = h, .cols = w,
	};
	struct ncplane* plane = ncplane_create(parent_plane, &plane_opts);
	if(box) ncplane_rounded_box_sized(plane, 0, 0, h, w, 0);

	int current_y = box ? 1 : 0;
	int current_x = box ? 1 : 0;
	if(pre_text){
		ncplane_putstr_yx(plane, current_y, current_x, pre_text);
		current_y++; //TBD assumes 1 line
	}

	int prompt_len = 0;
	if(prompt){
		ncplane_putstr_yx(plane, current_y, current_x, prompt);
		prompt_len = strlen(prompt);
	}

	struct ncreader_options ncreader_options = {.flags=NCREADER_OPTION_CURSOR};
	struct ncplane_options reader_plane_options = {
		.y = current_y, .x = current_x + prompt_len,
		.rows = 1, .cols = w - prompt_len - (box ? 2 : 0),
	};
	struct ncplane* reader_subplane = ncplane_create(plane, &reader_plane_options);
	struct ncreader* reader = ncreader_create(reader_subplane, &ncreader_options);
	struct ncinput reader_input;

	current_y++;
	if(post_text){
		ncplane_putstr_yx(plane, current_y, current_x, post_text);
	}

	uint32_t c;
	do{
		ncpile_render(plane);
		ncpile_rasterize(plane);
		c = notcurses_get(nc, NULL, &reader_input);
		if(c==NCKEY_ENTER) break;
		if(c==NCKEY_ESC){
			ncreader_destroy(reader, NULL);
			ncplane_destroy(plane);
			return NULL;
		}
		ncreader_offer_input(reader, &reader_input);
	}while(1);
	char* reader_contents;
	ncreader_destroy(reader, &reader_contents);
	ncplane_destroy(plane);
	return reader_contents;
}

char* centered_input_reader(struct ncplane* parent_plane, int h, int w, const char* pre_text, const char* post_text, const char* prompt, bool box){
	struct ncplane_options plane_opts = {
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.rows = h, .cols = w,
		.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED
	};
	struct ncplane* plane = ncplane_create(parent_plane, &plane_opts);
	if(box) ncplane_rounded_box_sized(plane, 0, 0, h, w, 0);

	int current_y = box ? 1 : 0;
	int current_x = box ? 1 : 0;
	if(pre_text){
		ncplane_putstr_yx(plane, current_y, current_x, pre_text);
		current_y++; //TBD assumes 1 line
	}

	int prompt_len = 0;
	if(prompt){
		ncplane_putstr_yx(plane, current_y, current_x, prompt);
		prompt_len = strlen(prompt);
	}

	struct ncreader_options ncreader_options = {.flags=NCREADER_OPTION_CURSOR};
	struct ncplane_options reader_plane_options = {
		.y = current_y, .x = current_x + prompt_len,
		.rows = 1, .cols = w - prompt_len - (box ? 2 : 0),
	};
	struct ncplane* reader_subplane = ncplane_create(plane, &reader_plane_options);
	struct ncreader* reader = ncreader_create(reader_subplane, &ncreader_options);
	struct ncinput reader_input;

	current_y++;
	if(post_text){
		ncplane_putstr_yx(plane, current_y, current_x, post_text);
	}

	uint32_t c;
	do{
		ncpile_render(plane);
		ncpile_rasterize(plane);
		c = notcurses_get(nc, NULL, &reader_input);
		if(c==NCKEY_ENTER) break;
		if(c==NCKEY_ESC){
			ncreader_destroy(reader, NULL);
			ncplane_destroy(plane);
			return NULL;
		}
		ncreader_offer_input(reader, &reader_input);
	}while(1);
	char* reader_contents;
	ncreader_destroy(reader, &reader_contents);
	ncplane_destroy(plane);
	return reader_contents;
}

static void add_tag_to_search(char exclude_flag, sqlite3_int64 tag_id){
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

static inline char tag_selected(sqlite3_int64 tag, sqlite3_int64* selected_tags, unsigned short selected_tags_n){
	for(unsigned short i=0; i<selected_tags_n; i++){
		if(selected_tags[i] == tag) return 1;
	}
	return 0;
}

static void add_tag_to_search_tui(){
	unsigned int plane_rows, plane_cols;
	notcurses_stddim_yx(nc, &plane_rows, &plane_cols);
	if(plane_rows>=TAG_SEARCH_ROWS+4) plane_rows = TAG_SEARCH_ROWS+4;
	if(plane_cols>=TAG_SEARCH_COLS) plane_cols = TAG_SEARCH_COLS;
	struct ncplane_options plane_options = {
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.rows = plane_rows, .cols = plane_cols,
		.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED,
	};
	struct ncplane* plane = ncplane_create(search_plane, &plane_options);
	plane_options.x = NCALIGN_RIGHT;
	struct ncplane* side_plane = ncplane_create(search_plane, &plane_options);

	char* tag_search = strdup("");
	struct id_dynarr search_results = new_id_dynarr(10);
	search_tags(&search_results, tag_search);
	char exclude_flag=0, side_plane_flag=0;
	sqlite3_int64* selected_tags = NULL;
	unsigned short selected_tags_n = 0;
	unsigned short ui_index=0, ui_elements=1, side_ui_index=0;
	struct ncinput reader_input;
	uint32_t c = NCKEY_RESIZE;

	do{
		//drawing logic
		ncplane_erase(plane);
		ncplane_erase(side_plane);
		ui_elements = search_results.used+1;
		if(ui_elements>plane_rows-3) ui_elements = plane_rows-3;
		
		ncplane_printf_yx(plane, 0, 2, "Search: %s", tag_search);

		//mark cursor position
		if(side_plane_flag){
			ncplane_putstr_yx(side_plane, 1+side_ui_index, 0, "->");
		}else if(ui_index==0){
			ncplane_putstr_yx(plane, 0, 0, "->");
		}else if(ui_elements>0){
			ncplane_putstr_yx(plane, 1+ui_index, 0, "->");
		}else{
			ui_index=0;
		}
		//print search results
		if(search_results.used==0){
			ncplane_putstr_yx(plane, 2, 2, "No tags found");
		}else{
			for(unsigned short i=0; i<search_results.used && i<(plane_rows-4); i++){
				ncplane_putstr_yx(plane, i+2, 2, tag_name_from_id(search_results.data[i]));
			}
		}
		//exclude flag
		if(exclude_flag){
			ncplane_putstr_yx(plane, plane_rows-1, 0, "Excluding");
			ncplane_format(plane, -1, 0, 1, 0, NCSTYLE_ITALIC);
		}
		ncpile_render(plane);
		ncpile_rasterize(plane);
		//side plane
		ncplane_putstr_yx(side_plane, 0, 2, "Selected tags:");
		if(selected_tags_n>0){
			for(unsigned short i=0; i<selected_tags_n; i++){
				ncplane_putstr_yx(side_plane, 1+i, 2, tag_name_from_id(selected_tags[i]));
			}
		}
		ncpile_render(side_plane);
		ncpile_rasterize(side_plane);

		c = notcurses_get(nc, NULL, &reader_input);

		if(!side_plane_flag && ui_index==0){
			if(c == NCKEY_BACKSPACE || c == NCKEY_DEL){
				int len = strlen(tag_search);
				if(len > 0){
					tag_search[len - 1] = '\0';
					search_tags(&search_results, tag_search);
				}
			} else if(isprint(reader_input.id)){
				int len = strlen(tag_search);
				tag_search = realloc(tag_search, len + strlen(reader_input.utf8) + 1);
				strcat(tag_search, reader_input.utf8);
				search_tags(&search_results, tag_search);
				continue;
			}
		}

		switch(c){
			case NCKEY_DOWN:
				if(side_plane_flag){
					if(side_ui_index<selected_tags_n-1) side_ui_index++;
					else side_ui_index = 0;
				}else{
					if(ui_index<ui_elements-1) ui_index++;
					else ui_index = 0;
				}
				break;
			case NCKEY_UP:
				if(side_plane_flag){
					if(side_ui_index>0) side_ui_index--;
					else side_ui_index = selected_tags_n-1;
				}else{
					if(ui_index>0) ui_index--;
					else ui_index = ui_elements-1;
				}
				break;
			case NCKEY_RIGHT:
			case NCKEY_LEFT:
				if(selected_tags_n>0){
					side_plane_flag = !side_plane_flag;
				}
				break;
			case 'g':
				ui_index = 0;
				side_plane_flag = false;
				break;
			case NCKEY_ENTER:
				if(!side_plane_flag){
					if(ui_index==0 && search_results.used > 0){
						add_tag_to_search(exclude_flag, search_results.data[0]);
						goto end_label;
					}else{
						add_tag_to_search(exclude_flag, search_results.data[ui_index-1]);
						goto end_label;
					}
				}
				break;
			case ' ':
				if(!side_plane_flag && ui_index>0 && !tag_selected(search_results.data[ui_index-1], selected_tags, selected_tags_n)){
					selected_tags_n++;
					selected_tags = realloc(selected_tags, sizeof(sqlite3_int64)*selected_tags_n);
					selected_tags[selected_tags_n-1] = search_results.data[ui_index-1];
				}
				break;
			case 'd': //delete tag from selected
				if(side_plane_flag){
					for(unsigned short i=side_ui_index+1; i<selected_tags_n; i++){
						selected_tags[i-1] = selected_tags[i];
					}
					selected_tags_n--;
					if(selected_tags_n==0) side_plane_flag=0;
					else if(side_ui_index>=selected_tags_n) side_ui_index--;
				}
				break;
			case 'a': //AND
				if(selected_tags_n>0){
					for(unsigned short i=0; i<selected_tags_n; i++){
						add_tag_to_search(exclude_flag, selected_tags[i]);
					}
					goto end_label;
				}
				break;
			case 'o': //OR
				if(selected_tags_n>0){
					if(selected_tags_n==1){
						add_tag_to_search(exclude_flag, selected_tags[0]);
					}else{
						search->or_tag_elements_n++;
						search->or_tag_elements = realloc(search->or_tag_elements, sizeof(struct or_tag_element)*search->or_tag_elements_n);
						search->or_tag_elements[search->or_tag_elements_n-1].or_number = selected_tags_n;
						search->or_tag_elements[search->or_tag_elements_n-1].ids = malloc(selected_tags_n*sizeof(sqlite3_int64));
						memcpy(search->or_tag_elements[search->or_tag_elements_n-1].ids, selected_tags, selected_tags_n*sizeof(sqlite3_int64));
						search_not_run=1;
					}
					goto end_label;
				}
				break;
			case 'A': //add all tags to selected
				if(search_results.used>0){
					unsigned short previous_size = selected_tags_n;
					selected_tags = realloc(selected_tags, sizeof(sqlite3_int64)*(selected_tags_n+search_results.used));
					for(unsigned short i=0; i<search_results.used; i++){
						if(!tag_selected(search_results.data[i], selected_tags, previous_size)){
							selected_tags[selected_tags_n] = search_results.data[i];
							selected_tags_n++;
						}
					}
				}
				break;
			case 'e': //flip exclude flag
				exclude_flag = !exclude_flag;
				break;
			case 'q':
			case NCKEY_ESC:
				goto end_label;
		}
	}while(c!='q' && c!='Q');
	end_label:
	ncplane_destroy(plane);
	ncplane_destroy(side_plane);
	if(tag_search) free(tag_search);
	if(search_results.data) free(search_results.data);
	if(selected_tags) free(selected_tags);
}

static unsigned short size_unit_from_ptr(char* ptr, unsigned short init_value){
	while(*ptr==' ') ptr++;	//skip whitespace
	if(*ptr=='B') return 0;
	if(*ptr=='K') return 1;
	if(*ptr=='M') return 2;
	if(*ptr=='G') return 3;
	return init_value;
}

static unsigned int calculate_tags_area_width(){
	unsigned int result = SEARCH_TUI_MIN_TAGS_AREA_WIDTH-2;
	for(unsigned short i=0; i<search->or_tag_elements_n; i++){
		unsigned short or_tag_elements_n = search->or_tag_elements[i].or_number;
		unsigned int width = (or_tag_elements_n-1)*4;
		for(unsigned short j=0; j<or_tag_elements_n; j++){
			width += strlen(tag_name_from_id(search->or_tag_elements[i].ids[j]));
		}
		if(width>result) result=width;
	}
	for(unsigned short i=0; i<search->exclude_tags_n; i++){
		unsigned int width = strlen(tag_name_from_id(search->exclude_tags[i]))+1;
		if(width>result) result=width;
	}
	for(unsigned short i=0; i<search->include_tags_n; i++){
		unsigned int width = strlen(tag_name_from_id(search->include_tags[i]));
		if(width>result) result=width;
	}
	return result+2;
}

static void add_filepath_expression_to_search_tui(){
	char* options[] = {"Include", "Exclude", NULL};
	int choice = chooser(search_plane, options, -1);
	if(choice == -1) return;

	unsigned int plane_rows, plane_cols;
	notcurses_stddim_yx(nc, &plane_rows, &plane_cols);

	char* expression = centered_input_reader(search_plane, 4, 40, "Enter filepath expression", NULL, "> ", true);
	if(expression == NULL || expression[0] == '\0'){
		if(expression) free(expression);
		return;
	}

	if(choice == 0){ //include
		search->include_filepaths_n++;
		search->include_filepaths = realloc(search->include_filepaths, sizeof(char*) * search->include_filepaths_n);
		search->include_filepaths[search->include_filepaths_n - 1] = expression;
	} else { //exclude
		search->exclude_filepaths_n++;
		search->exclude_filepaths = realloc(search->exclude_filepaths, sizeof(char*) * search->exclude_filepaths_n);
		search->exclude_filepaths[search->exclude_filepaths_n - 1] = expression;
	}
	search_not_run = 1;
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
	FILE* log_fp = freopen(INIT_DIRECTORY"/tui.log", "w", stderr);	//redirect stderr to log file

	load_tui_options(INIT_DIRECTORY"/""tui_options");
	if(flags & START_TUI_DISPLAY){
		fullscreen_display((struct search*)data);
		goto end_label;
	}
	new_search_plane((struct search*)data);

	unsigned int screen_rows, screen_cols;
	notcurses_stddim_yx(nc, &screen_rows, &screen_cols);
	enum {byte, kilobyte, megabyte, gigabyte} min_size_unit=2, max_size_unit=2;
	unsigned long long min_size=search->min_size/1000000, max_size=search->max_size/1000000;
	char min_size_unit_str[3], max_size_unit_str[3];
	char* reader_result, * size_unit_ptr;
	unsigned short ui_index = 0;
	unsigned short tag_elements = search->include_tags_n+search->exclude_tags_n+search->or_tag_elements_n;
	unsigned short filepath_elements = search->include_filepaths_n+search->exclude_filepaths_n+search->or_filepath_elements_n;
	unsigned short active_ui_element_lines = SEARCH_TUI_BEGINNING_ELEMENTS + 1 + tag_elements;
	unsigned short ui_element_lines;
	if(tag_elements>filepath_elements) ui_element_lines = SEARCH_TUI_BEGINNING_ELEMENTS + 1 + tag_elements;
	else ui_element_lines = SEARCH_TUI_BEGINNING_ELEMENTS + 1 + filepath_elements;
	unsigned int tags_area_width = calculate_tags_area_width();
	bool left = true;
	short export = -1;
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
				if(ui_index<active_ui_element_lines-1) ui_index++;
				else ui_index = 0;
				break;
			case NCKEY_UP:
				if(ui_index>0) ui_index--;
				else ui_index = active_ui_element_lines-1;
				break;
			case NCKEY_LEFT:
			case NCKEY_RIGHT:
				left = !left;
				if(left) active_ui_element_lines=SEARCH_TUI_BEGINNING_ELEMENTS + 1 + tag_elements;
				else active_ui_element_lines=SEARCH_TUI_BEGINNING_ELEMENTS + 1 + filepath_elements;
				if(ui_index>=active_ui_element_lines) ui_index = active_ui_element_lines-1;
				break;
			case 'g':
				ui_index = 0;
				break;
			case 'G':
				ui_index = active_ui_element_lines-1;
				break;
			case 't':
				add_tag_to_search_tui();
				break;
			case 'o':
				options_tui();
				break;
			case 'e': //export search
				char* export_options[] = {"Search result paths", "Search result ids", "SQL query", NULL};
				export = chooser(search_plane, export_options, -1);
				if(export!=-1) goto end_label; //TBD ask for confirmation
				break;
			case 'd':	//delete tag or filepath expression from search list
				if(ui_index>SEARCH_TUI_BEGINNING_ELEMENTS){
					if(left){
						//delete tag
						unsigned short tag_n = ui_index-SEARCH_TUI_BEGINNING_ELEMENTS-1;
						unsigned int tag_element_length;
						if(tag_n<search->include_tags_n){
							if(tags_area_width>SEARCH_TUI_MIN_TAGS_AREA_WIDTH) tag_element_length = strlen(tag_name_from_id(search->include_tags[tag_n]));
							while(tag_n<search->include_tags_n){
								search->include_tags[tag_n] = search->include_tags[tag_n+1];
								tag_n++;
							}
							search->include_tags_n-=1;
							search->include_tags = realloc(search->include_tags, sizeof(sqlite3_int64)*search->include_tags_n);
						}else{
							tag_n-=search->include_tags_n;
							if(tag_n<search->exclude_tags_n){
								if(tags_area_width>SEARCH_TUI_MIN_TAGS_AREA_WIDTH) tag_element_length = strlen(tag_name_from_id(search->exclude_tags[tag_n]))+1;
								while(tag_n<search->exclude_tags_n){
									search->exclude_tags[tag_n] = search->exclude_tags[tag_n+1];
									tag_n++;
								}
								search->exclude_tags_n-=1;
								search->exclude_tags = realloc(search->exclude_tags, sizeof(sqlite3_int64)*search->exclude_tags_n);
							}else{
								tag_n-=search->exclude_tags_n;
								if(tags_area_width>SEARCH_TUI_MIN_TAGS_AREA_WIDTH){
									unsigned short or_n = search->or_tag_elements[tag_n].or_number;
									tag_element_length = (or_n-1)*4;
									for(unsigned short j=0; j<or_n; j++){
										tag_element_length += strlen(tag_name_from_id(search->or_tag_elements[tag_n].ids[j]));
									}
								}
								free(search->or_tag_elements[tag_n].ids);
								unsigned short or_tag_elements_n = search->or_tag_elements_n;
								while(tag_n<or_tag_elements_n){
									search->or_tag_elements[tag_n] = search->or_tag_elements[tag_n+1];
									tag_n++;
								}
								search->or_tag_elements_n-=1;
								search->or_tag_elements = realloc(search->or_tag_elements, sizeof(struct or_tag_element)*search->or_tag_elements_n);
							}
						}
						if(ui_index == active_ui_element_lines-1) ui_index--;
						if(tags_area_width>SEARCH_TUI_MIN_TAGS_AREA_WIDTH && tag_element_length==tags_area_width-2) tags_area_width = calculate_tags_area_width();
					}else{
						//delete filepath expression
						unsigned short filepath_n = ui_index - SEARCH_TUI_BEGINNING_ELEMENTS - 1;
						if (filepath_n < search->include_filepaths_n) {
							free(search->include_filepaths[filepath_n]);
							for (unsigned short i = filepath_n; i < search->include_filepaths_n - 1; i++) {
								search->include_filepaths[i] = search->include_filepaths[i + 1];
							}
							search->include_filepaths_n--;
							if (search->include_filepaths_n > 0) {
								search->include_filepaths = realloc(search->include_filepaths, sizeof(char*) * search->include_filepaths_n);
							} else {
								free(search->include_filepaths);
								search->include_filepaths = NULL;
							}
						} else {
							filepath_n -= search->include_filepaths_n;
							if (filepath_n < search->exclude_filepaths_n) {
								free(search->exclude_filepaths[filepath_n]);
								for (unsigned short i = filepath_n; i < search->exclude_filepaths_n - 1; i++) {
									search->exclude_filepaths[i] = search->exclude_filepaths[i + 1];
								}
								search->exclude_filepaths_n--;
								if (search->exclude_filepaths_n > 0) {
									search->exclude_filepaths = realloc(search->exclude_filepaths, sizeof(char*) * search->exclude_filepaths_n);
								} else {
									free(search->exclude_filepaths);
									search->exclude_filepaths = NULL;
								}
							} else {
								filepath_n -= search->exclude_filepaths_n;
								struct or_filepath_element* or_element = &search->or_filepath_elements[filepath_n];
								for (unsigned short i = 0; i < or_element->or_number; i++) {
									free(or_element->patterns[i]);
								}
								free(or_element->patterns);

								for (unsigned short i = filepath_n; i < search->or_filepath_elements_n - 1; i++) {
									search->or_filepath_elements[i] = search->or_filepath_elements[i + 1];
								}
								search->or_filepath_elements_n--;
								if (search->or_filepath_elements_n > 0) {
									search->or_filepath_elements = realloc(search->or_filepath_elements, sizeof(struct or_filepath_element) * search->or_filepath_elements_n);
								} else {
									free(search->or_filepath_elements);
									search->or_filepath_elements = NULL;
								}
							}
						}
						if (ui_index == active_ui_element_lines - 1) ui_index--;
						search_not_run = 1;
					}
				}
				break;
			case NCKEY_ENTER:
			case ' ':
				switch(ui_index){
					case 0:	//order by
						char* order_options[] = {"None", "Size", "Import time", "Random", NULL};
						search->order_by = chooser(search_plane, order_options, search->order_by);
						if(search->order_by==none || search->order_by==random_order){
							search->descending = 0;
						}else{
							if(search->order_by == import_time){
								char* descending_options[] = {"Oldest first", "Newest first", NULL};
								search->descending = chooser(search_plane, descending_options, search->descending);
							}else{
								char* descending_options[] = {"Ascending", "Descending", NULL};
								search->descending = chooser(search_plane, descending_options, search->descending);
							}
						}
						search_not_run = 1;
						break;
					case 1:	//limit
						reader_result = input_reader(search_plane, 1+ui_index, 23, 1, 12, NULL, NULL, NULL, false);
						if(reader_result != NULL){
							search->limit = strtol(reader_result, NULL, 10);
							free(reader_result);
						}
						search_not_run = 1;
						break;
					case 2:	//min_size
						reader_result = input_reader(search_plane, 1+ui_index, 13, 1, 18, NULL, NULL, NULL, false);
						if(reader_result != NULL){
							min_size = strtoull(reader_result, &size_unit_ptr, 10);
							min_size_unit = size_unit_from_ptr(size_unit_ptr, min_size_unit);
							free(reader_result);
						}
						search_not_run = 1;
						break;
					case 3:	//max_size
						reader_result = input_reader(search_plane, 1+ui_index, 26, 1, 18, NULL, NULL, NULL, false);
						if(reader_result != NULL){
							max_size = strtoull(reader_result, &size_unit_ptr, 10);
							max_size_unit = size_unit_from_ptr(size_unit_ptr, max_size_unit);
							free(reader_result);
						}
						search_not_run = 1;
						break;
					case 4: //filetypes
						char* filetype_options[] = {"Image", "Video", "Other", NULL};
						search->filetypes = multiple_chooser(search_plane, filetype_options, search->filetypes);
						search_not_run = 1;
						break;
					case 5:	//add tag or filepath expression to search
						if(left){
							//tag
							add_tag_to_search_tui();
							tags_area_width = calculate_tags_area_width(); //TBD optimize this, not always necessary to recalculate
						}else{
							//filepath expression
							add_filepath_expression_to_search_tui();
						}
						break;
					default:
						//TBD modify search element
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

		//calculate number of elements and sizes
		tag_elements = search->include_tags_n + search->exclude_tags_n + search->or_tag_elements_n;
		filepath_elements = search->include_filepaths_n + search->exclude_filepaths_n + search->or_filepath_elements_n;
		if(left){
			active_ui_element_lines = SEARCH_TUI_BEGINNING_ELEMENTS + 1 + tag_elements;
		}else{
			active_ui_element_lines = SEARCH_TUI_BEGINNING_ELEMENTS + 1 + filepath_elements;
		}
		if(tag_elements>filepath_elements) ui_element_lines = SEARCH_TUI_BEGINNING_ELEMENTS + 1 + tag_elements;
		else ui_element_lines = SEARCH_TUI_BEGINNING_ELEMENTS + 1 + filepath_elements;
		compose_search_sql(search);
		ncplane_erase(search_plane);
		//mark current index
		if(ui_index<SEARCH_TUI_BEGINNING_ELEMENTS) ncplane_putstr_yx(search_plane, 1+ui_index, 0, "->");
		else if(ui_index==SEARCH_TUI_BEGINNING_ELEMENTS){
			if(left) ncplane_putstr_yx(search_plane, SEARCH_TUI_BEGINNING_ELEMENTS+2, 0, "->");
			else ncplane_putstr_yx(search_plane, SEARCH_TUI_BEGINNING_ELEMENTS+2, tags_area_width, "->");
		}else{
			if(left) ncplane_putstr_yx(search_plane, 2+ui_index, 0, "->");
			else ncplane_putstr_yx(search_plane, 2+ui_index, tags_area_width, "->");
		}
		//number of results
		ncplane_printf_yx(search_plane, 0, 0, "Results: %ld", search->output_ids.used);
		if(search_not_run){
			ncplane_putstr(search_plane, " (search not run)");
			ncplane_cursor_move_rel(search_plane, 0, -(int)strlen("(search not run)"));
			ncplane_format(search_plane, 0, -1, 1, 0, NCSTYLE_ITALIC);
		}
		//order by
		ncplane_putstr_yx(search_plane, 1, 3, "Order by: ");
		switch(search->order_by){
			case none:
				ncplane_putstr(search_plane, "none");
				break;
			case size:
				ncplane_putstr(search_plane, "size");
				break;
			case import_time:
				ncplane_putstr(search_plane, "import time");
				break;
			case random_order:
				ncplane_putstr(search_plane, "random");
				break;
		}
		if(search->order_by!=none && search->order_by!=random_order){
			if(search->order_by == import_time){
				if(search->descending) ncplane_putstr(search_plane, " newest first");
				else ncplane_putstr(search_plane, " oldest first");
			} else {
				if(search->descending) ncplane_putstr(search_plane, " descending");
				else ncplane_putstr(search_plane, " ascending");
			}
		}
		//limit
		ncplane_printf_yx(search_plane, 2, 3, "Limit (0 for none): %lu", search->limit);
		//min, max size
		ncplane_printf_yx(search_plane, 3, 3, "Min size: %llu %s", min_size, min_size_unit_str);
		ncplane_printf_yx(search_plane, 4, 3, "Max size (0 for none): %llu %s", max_size, max_size_unit_str);
		//filetypes
		ncplane_putstr_yx(search_plane, 5, 3, "Filetype: ");
		if(search->filetypes==0 || search->filetypes==(FILETYPE_IMAGE|FILETYPE_VIDEO|FILETYPE_OTHER)) ncplane_putstr(search_plane, "Any");
		else{
			unsigned short matches=0;
			if(search->filetypes&(FILETYPE_IMAGE)){ncplane_putstr(search_plane, "Image"); matches++;}
			if(search->filetypes&(FILETYPE_VIDEO)){if(matches)ncplane_putstr(search_plane, " or "); ncplane_putstr(search_plane, "Video"); matches++;}
			if(search->filetypes&(FILETYPE_OTHER)){if(matches)ncplane_putstr(search_plane, " or "); ncplane_putstr(search_plane, "Other"); matches++;}
		}
		//add new tag button
		ncplane_putstr_yx(search_plane, SEARCH_TUI_BEGINNING_ELEMENTS+2, 3, "Add new tag to search");
		//add new filepath expression button
		ncplane_putstr_yx(search_plane, SEARCH_TUI_BEGINNING_ELEMENTS+2, tags_area_width+3, "Add new filepath expression to search");
		//tags
		for(unsigned short i=0; i<search->include_tags_n; i++){
			ncplane_putstr_yx(search_plane, 3+SEARCH_TUI_BEGINNING_ELEMENTS+i, 3, tag_name_from_id(search->include_tags[i]));
		}
		for(unsigned short i=0; i<search->exclude_tags_n; i++){
			ncplane_printf_yx(search_plane, 3+SEARCH_TUI_BEGINNING_ELEMENTS+search->include_tags_n+i, 3, "-%s", tag_name_from_id(search->exclude_tags[i]));
		}
		for(unsigned short i=0; i<search->or_tag_elements_n; i++){
			ncplane_cursor_move_yx(search_plane, 3+SEARCH_TUI_BEGINNING_ELEMENTS+search->include_tags_n+search->exclude_tags_n+i, 3);
			for(unsigned short j=0; j<search->or_tag_elements[i].or_number; j++){
				ncplane_putstr(search_plane, tag_name_from_id(search->or_tag_elements[i].ids[j]));
				if(j<search->or_tag_elements[i].or_number-1) ncplane_putstr(search_plane, " OR ");
			}
		}
		//filepath expressions
		for(unsigned short i=0; i<search->include_filepaths_n; i++){
			ncplane_putstr_yx(search_plane, 3+SEARCH_TUI_BEGINNING_ELEMENTS+i, tags_area_width+3, search->include_filepaths[i]);
		}
		for(unsigned short i=0; i<search->exclude_filepaths_n; i++){
			ncplane_printf_yx(search_plane, 3+SEARCH_TUI_BEGINNING_ELEMENTS+search->include_filepaths_n+i, tags_area_width+3, "-%s", search->exclude_filepaths[i]);
		}
		for(unsigned short i=0; i<search->or_filepath_elements_n; i++){
			ncplane_cursor_move_yx(search_plane, 3+SEARCH_TUI_BEGINNING_ELEMENTS+search->include_filepaths_n+search->exclude_filepaths_n+i, tags_area_width+3);
			for(unsigned short j=0; j<search->or_filepath_elements[i].or_number; j++){
				ncplane_putstr(search_plane, search->or_filepath_elements[i].patterns[j]);
				if(j<search->or_filepath_elements[i].or_number-1) ncplane_putstr(search_plane, " OR ");
			}
		}
		//SQL query
		ncplane_printf_yx(search_plane, ui_element_lines+4, 0, "SQL query: %s", search->sql);
		ncpile_render(search_plane);
		ncpile_rasterize(search_plane);
	}while((c=notcurses_get(nc, NULL, NULL))!='Q');

	end_label:
	for(unsigned short i=0; i<tui_options.shortcuts_n; i++){
		if(tui_options.shortcuts[i].string) free(tui_options.shortcuts[i].string);
	}
	free(tui_options.shortcuts);
	ncplane_destroy(search_plane);
	notcurses_drop_planes(nc);
	notcurses_stop(nc);
	if(export==0){
		for(unsigned short i=0; i<search->output_ids.used; i++){
			char* output_path = transform_output_path(filepath_from_id(search->output_ids.data[i]));
			printf("%s\n", output_path);
			free(output_path);
		}
	}else if(export==1){
		for(unsigned short i=0; i<search->output_ids.used; i++){
			printf("%lld\n", search->output_ids.data[i]);
		}
	}else if(export==2){
		printf("%s\n", search->sql);
	}
	free_search(search);
	free(search);
	fclose(log_fp);
}

static sqlite3_int64 add_tag_to_file_tui(struct ncplane* parent_plane){
	unsigned int plane_rows, plane_cols;
	notcurses_stddim_yx(nc, &plane_rows, &plane_cols);
	if(plane_rows>=TAG_SEARCH_ROWS+4) plane_rows = TAG_SEARCH_ROWS+4;
	if(plane_cols>=TAG_SEARCH_COLS) plane_cols = TAG_SEARCH_COLS;
	struct ncplane_options plane_options = {
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.rows = plane_rows, .cols = plane_cols,
		.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED
	};
	struct ncplane* plane = ncplane_create(parent_plane, &plane_options);
	sqlite3_int64 tag_id = -1;
	char* tag_search = NULL;
	struct id_dynarr search_results = new_id_dynarr(10);
	unsigned short ui_index = 0;
	uint32_t c = NCKEY_ENTER;
	do{
		switch(c){
			case NCKEY_DOWN:
				if(ui_index<search_results.used) ui_index++;
				else ui_index = 0;
				break;
			case NCKEY_UP:
				if(ui_index>0) ui_index--;
				else ui_index = search_results.used;
				break;
			case 'n':
				add_tag(tag_search);
				search_tags(&search_results, tag_search);
				break;
			case NCKEY_ENTER:
				if(ui_index==0){
					if(tag_search!=NULL) free(tag_search);
					ncplane_putstr_yx(plane, 0, 2, "Type to search:");
					tag_search = input_reader(plane, 1, 2, 1, plane_cols-2, NULL, NULL, NULL, false);
					if(tag_search != NULL) search_tags(&search_results, tag_search);
				}else{
					tag_id = search_results.data[ui_index-1];
					goto end_flag;
				}
				break;
		}
		ncplane_erase(plane);
		ncplane_putstr_yx(plane, 0, 2, "Searching for:");
		ncplane_putstr_yx(plane, 1, 2, tag_search);
		ncplane_putstr_yx(plane, 3, 2, "Search results:");
		if(search_results.used==0){
			ncplane_putstr_yx(plane, 4, 2, "No tag found, press 'n' to create new tag");
		}else{
			for(unsigned short i=0; i<search_results.used; i++){
				ncplane_putstr_yx(plane, 4+i, 2, tag_name_from_id(search_results.data[i]));
			}
		}
		//mark current index
		if(ui_index==0) ncplane_putstr_yx(plane, 1, 0, "->");
		else ncplane_putstr_yx(plane, 3+ui_index, 0, "->");
		ncpile_render(plane);
		ncpile_rasterize(plane);
	}while((c=notcurses_get(nc, NULL, NULL))!='q');
	end_flag:
	if(tag_search!=NULL) free(tag_search);
	if(search_results.data!=NULL) free(search_results.data);
	ncplane_destroy(plane);
	return tag_id;
}

void file_tag_tui(sqlite3_int64 id){
	unsigned int screen_rows, screen_cols;
	notcurses_stddim_yx(nc, &screen_rows, &screen_cols);
	struct ncplane_options plane_options = {
		.y = 0, .x = 0,
		.rows = screen_rows, .cols = screen_cols,
	};
	struct ncplane* plane = ncpile_create(nc, &plane_options);

	sqlite3_int64 new_tag_id = -1;
	struct id_dynarr file_tags = new_id_dynarr(10);
	unsigned short ui_index = 0;
	uint32_t c = NCKEY_RESIZE;
	do{
		switch(c){
			case NCKEY_DOWN:
				if(ui_index<file_tags.used) ui_index++;
				else ui_index = 0;
				break;
			case NCKEY_UP:
				if(ui_index>0) ui_index--;
				else ui_index = file_tags.used;
				break;
			case 'd':
				if(ui_index>0){
					untag(id, file_tags.data[ui_index-1]);
					search_not_run = 1;
					if(ui_index==file_tags.used) ui_index--;
				}
				break;
			case NCKEY_ENTER:
				if(ui_index==0){
					case 't':
					new_tag_id = add_tag_to_file_tui(plane);
					if(new_tag_id!=-1){
						tag(id, new_tag_id);
						search_not_run = 1;
					}
					break;
				}
				break;
		}
		get_file_tags(id, &file_tags);
		//redraw plane
		ncplane_erase(plane);
		ncplane_putstr_yx(plane, 0, 2, "Add new tag");
		ncplane_putstr_yx(plane, 3, 2, "Current tags:");
		for(unsigned short j=0; j<file_tags.used; j++){
			ncplane_putstr_yx(plane, 5+j, 2, tag_name_from_id(file_tags.data[j]));
		}
		//mark current index
		if(ui_index==0) ncplane_putstr_yx(plane, 0, 0, "->");
		else ncplane_putstr_yx(plane, 4+ui_index, 0, "->");
		ncpile_render(plane);
		ncpile_rasterize(plane);
	}while((c=notcurses_get(nc, NULL, NULL))!='q');

	free(file_tags.data);
	ncplane_destroy(plane);
}
