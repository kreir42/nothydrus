#include "nothydrus.h"
#include "tui.h"

static uint32_t ask_for_key(struct ncplane* parent_plane){
	struct ncplane_options plane_options = {
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.rows = 3, .cols = 30,
		.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED,
	};
	struct ncplane* plane = ncplane_create(parent_plane, &plane_options);
	ncplane_putstr_yx(plane, 1, 1, "Press a key: ");
	ncpile_render(plane);
	ncpile_rasterize(plane);
	uint32_t key = notcurses_get(nc, NULL, NULL);
	ncplane_destroy(plane);
	return key;
}

static char* ask_for_command(struct ncplane* parent_plane, unsigned short parent_cols){
	unsigned short plane_cols = parent_cols/2;
	struct ncplane_options plane_options = {
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.rows = 3, .cols = plane_cols,
		.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED,
	};
	struct ncplane* plane = ncplane_create(parent_plane, &plane_options);
	ncplane_putstr_yx(plane, 0, 1, "Write the shell command: ");
	ncpile_render(plane);
	ncpile_rasterize(plane);
	char* string = input_reader(plane, 1, 1, 1, plane_cols-2); //TBD add prompt
	ncplane_destroy(plane);
	return string;
}

static short choose_custom_column(struct ncplane* plane){
	char** options = malloc(sizeof(char*)*custom_columns_n);
	for(unsigned short i=0; i<custom_columns_n; i++){
		options[i] = custom_columns[i].name;
	}
	short choice = chooser(plane, options, -1);
	free(options);
	return choice;
}

void options_tui(){
	unsigned int screen_rows, screen_cols;
	notcurses_stddim_yx(nc, &screen_rows, &screen_cols);
	struct ncplane_options plane_options = {
		.y = 0, .x = 0,
		.rows = screen_rows, .cols = screen_cols,
	};
	struct ncplane* plane = ncpile_create(nc, &plane_options);

	unsigned short ui_index = 0, ui_elements = OPTIONS_TUI_MIN_ELEMENTS + tui_options.shortcuts_n;
	uint32_t c = NCKEY_RESIZE;
	do{
		switch(c){
			case NCKEY_DOWN:
				if(ui_index<ui_elements-1) ui_index++;
				else ui_index = 0;
				break;
			case NCKEY_UP:
				if(ui_index>0) ui_index--;
				else ui_index = ui_elements-1;
				break;
			case 'd':
				if(ui_index>=OPTIONS_TUI_MIN_ELEMENTS){
					tui_options.shortcuts_n--;
					for(unsigned short i=ui_index-OPTIONS_TUI_MIN_ELEMENTS; i<tui_options.shortcuts_n; i++){
						tui_options.shortcuts[i] = tui_options.shortcuts[i+1];
					}
					tui_options.shortcuts = realloc(tui_options.shortcuts, sizeof(struct shortcut)*tui_options.shortcuts_n);
					if(tui_options.shortcuts_n==0 || ui_index==OPTIONS_TUI_MIN_ELEMENTS+tui_options.shortcuts_n) ui_index--;
				}
				break;
			case '+':
				switch(ui_index){
					case 1: tui_options.search_limit++; break;
				}
				break;
			case '-':
				switch(ui_index){
					case 1: if(tui_options.search_limit>0) tui_options.search_limit--; break;
				}
				break;
			case NCKEY_ENTER:
				switch(ui_index){
					case 0:
						char* order_options[] = {"None", "Size", "Random", NULL};
						tui_options.search_order_by = chooser(plane, order_options, tui_options.search_order_by);
						if(tui_options.search_order_by==none || tui_options.search_order_by==random_order){
							tui_options.search_descending = 0;
						}else{
							char* descending_options[] = {"Ascending", "Descending", NULL};
							tui_options.search_descending = chooser(plane, descending_options, tui_options.search_descending);
						}
						break;
					case 1:
						char* limit_reader_result = input_reader(plane, 1, 3+strlen("Default search limit (0 for none): "), 1, screen_cols-(3+strlen("Default search limit (0 for none): ")+2));
						tui_options.search_limit = strtol(limit_reader_result, NULL, 10);
						free(limit_reader_result);
						break;
					case 2:
						struct shortcut shortcut = {};
						shortcut.key = ask_for_key(plane);
						char* shortcut_options[] = {"Tag file", "Untag file", "Tag/untag file", "Increase custom column value", "Decrease custom column value", "Remove custom column value", "External shell command on file", NULL};
						short choice = chooser(plane, shortcut_options, -1);
						if(choice==-1) break;
						shortcut.type = choice;
						switch(shortcut.type){
							case SHORTCUT_TYPE_TAG_FILE:
							case SHORTCUT_TYPE_UNTAG_FILE:
							case SHORTCUT_TYPE_TAG_UNTAG_FILE:
								shortcut.id = search_tag_tui();
								break;
							case SHORTCUT_TYPE_CUSTOM_COLUMN_INCREASE:
							case SHORTCUT_TYPE_CUSTOM_COLUMN_DECREASE:
							case SHORTCUT_TYPE_CUSTOM_COLUMN_REMOVE:
								shortcut.id = choose_custom_column(plane);
								break;
							case SHORTCUT_TYPE_EXTERNAL_COMMAND:
								shortcut.string = ask_for_command(plane, screen_cols);
								if(shortcut.string==NULL) shortcut.id = -1;
								break;
						}
						if(shortcut.id!=-1){
							tui_options.shortcuts_n++;
							tui_options.shortcuts = realloc(tui_options.shortcuts, sizeof(struct shortcut)*tui_options.shortcuts_n);
							tui_options.shortcuts[tui_options.shortcuts_n-1] = shortcut;
							//TBD check for conflicts
						}
						break;
				}
				break;
		}
		ui_elements = OPTIONS_TUI_MIN_ELEMENTS + tui_options.shortcuts_n;
		ncplane_erase(plane);

		//mark current index
		if(ui_index<OPTIONS_TUI_MIN_ELEMENTS-1) ncplane_putstr_yx(plane, ui_index, 0, "->");
		else ncplane_putstr_yx(plane, 2+ui_index, 0, "->");
		//order by
		ncplane_putstr_yx(plane, 0, 3, "Default search order: ");
		switch(tui_options.search_order_by){
			case none:
				ncplane_putstr(plane, "none");
				break;
			case size:
				ncplane_putstr(plane, "size");
				break;
			case random_order:
				ncplane_putstr(plane, "random");
				break;
		}
		if(tui_options.search_order_by!=none && tui_options.search_order_by!=random_order){
			if(tui_options.search_descending) ncplane_putstr(plane, " descending");
			else ncplane_putstr(plane, " ascending");
		}
		ncplane_printf_yx(plane, 1, 3, "Default search limit (0 for none): %lu", tui_options.search_limit);	//limit
		ncplane_putstr_yx(plane, OPTIONS_TUI_MIN_ELEMENTS+1, 3, "Add new shortcut");
		for(unsigned short i=0; i<tui_options.shortcuts_n; i++){
			ncplane_printf_yx(plane, OPTIONS_TUI_MIN_ELEMENTS+2+i, 3, "%c --> ", tui_options.shortcuts[i].key);
			switch(tui_options.shortcuts[i].type){
				case SHORTCUT_TYPE_TAG_FILE:
					ncplane_putstr(plane, "Add tag to file: ");
					ncplane_putstr(plane, tag_fullname_from_id(tui_options.shortcuts[i].id));
					break;
				case SHORTCUT_TYPE_UNTAG_FILE:
					ncplane_putstr(plane, "Remove tag from file: ");
					ncplane_putstr(plane, tag_fullname_from_id(tui_options.shortcuts[i].id));
					break;
				case SHORTCUT_TYPE_TAG_UNTAG_FILE:
					ncplane_putstr(plane, "Add or remove tag to file: ");
					ncplane_putstr(plane, tag_fullname_from_id(tui_options.shortcuts[i].id));
					break;
				case SHORTCUT_TYPE_CUSTOM_COLUMN_INCREASE:
					ncplane_putstr(plane, "Increase value of custom column: ");
					ncplane_putstr(plane, custom_columns[tui_options.shortcuts[i].id].name);
					break;
				case SHORTCUT_TYPE_CUSTOM_COLUMN_DECREASE:
					ncplane_putstr(plane, "Decrease value of custom column: ");
					ncplane_putstr(plane, custom_columns[tui_options.shortcuts[i].id].name);
					break;
				case SHORTCUT_TYPE_CUSTOM_COLUMN_REMOVE:
					ncplane_putstr(plane, "Remove value of custom column: ");
					ncplane_putstr(plane, custom_columns[tui_options.shortcuts[i].id].name);
					break;
				case SHORTCUT_TYPE_EXTERNAL_COMMAND:
					ncplane_putstr(plane, "External command: ");
					ncplane_putstr(plane, tui_options.shortcuts[i].string);
					break;
			}
		}
		ncpile_render(plane);
		ncpile_rasterize(plane);
	}while((c=notcurses_get(nc, NULL, NULL))!='q');

	ncplane_destroy(plane);
	save_tui_options(INIT_DIRECTORY"/""tui_options");
}
