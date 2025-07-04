#include "nothydrus.h"
#include "tui.h"

static uint32_t ask_for_key(struct ncplane* parent_plane){
	struct ncplane_options plane_options = {
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.rows = 3, .cols = 40,
		.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED,
	};
	struct ncplane* plane = ncplane_create(parent_plane, &plane_options);
	ncplane_putstr_yx(plane, 1, 1, "Press any key, or ESC to cancel:");
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
	ncplane_putstr_yx(plane, 0, 1, "Write the shell command:");
	ncpile_render(plane);
	ncpile_rasterize(plane);
	char* string = input_reader(plane, 1, 1, 1, plane_cols-2); //TBD add prompt
	ncplane_destroy(plane);
	return string;
}

static char* ask_for_mode_name(struct ncplane* parent_plane){
	struct ncplane_options plane_options = {
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.rows = 3, .cols = 34,
		.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED,
	};
	struct ncplane* plane = ncplane_create(parent_plane, &plane_options);
	ncplane_putstr_yx(plane, 0, 1, "Write new shortcut mode name:");
	ncpile_render(plane);
	ncpile_rasterize(plane);
	char* string = input_reader(plane, 1, 1, 1, 32);
	ncplane_destroy(plane);
	return string;
}

static char* ask_for_custom_column_value(struct ncplane* parent_plane){
	struct ncplane_options plane_options = {
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.rows = 3, .cols = 16,
		.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED,
	};
	struct ncplane* plane = ncplane_create(parent_plane, &plane_options);
	ncplane_putstr_yx(plane, 0, 1, "Enter value:");
	ncpile_render(plane);
	ncpile_rasterize(plane);
	char* string = input_reader(plane, 1, 1, 1, 14);
	ncplane_destroy(plane);
	return string;
}

static short choose_custom_column(struct ncplane* plane){
	log_debug("Asking user to choose a custom column\n");
	char** options = malloc(sizeof(char*)*custom_columns_n);
	for(unsigned short i=0; i<custom_columns_n; i++){
		options[i] = custom_columns[i].name;
	}
	options[custom_columns_n] = NULL;
	short choice = chooser(plane, options, -1);
	free(options);
	return choice;
}

void delete_shortcut(unsigned short index){
	log_debug("Deleting shortcut with index %d\n", index);
	tui_options.shortcuts_n--;
	for(unsigned short i=index; i<tui_options.shortcuts_n; i++){
		tui_options.shortcuts[i] = tui_options.shortcuts[i+1];
	}
	tui_options.shortcuts = realloc(tui_options.shortcuts, sizeof(struct shortcut)*tui_options.shortcuts_n);
}

void options_tui(){
	unsigned int screen_rows, screen_cols;
	notcurses_stddim_yx(nc, &screen_rows, &screen_cols);
	struct ncplane_options plane_options = {
		.y = 0, .x = 0,
		.rows = screen_rows, .cols = screen_cols,
	};
	struct ncplane* plane = ncpile_create(nc, &plane_options);

	unsigned short ui_index = 0, ui_elements = OPTIONS_TUI_BEGINNING_ELEMENTS + tui_options.modes_n + 1 + tui_options.shortcuts_n;
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
				if(ui_index>=OPTIONS_TUI_BEGINNING_ELEMENTS && ui_index<OPTIONS_TUI_BEGINNING_ELEMENTS+tui_options.modes_n){
					//delete shortcut mode
					unsigned short mode_index_to_delete = ui_index - OPTIONS_TUI_BEGINNING_ELEMENTS;
					tui_options.modes_n--;
					for(unsigned short i=mode_index_to_delete; i<tui_options.modes_n; i++){
						tui_options.modes[i] = tui_options.modes[i+1];
					}
					tui_options.modes = realloc(tui_options.modes, sizeof(char*)*(tui_options.modes_n+1));
					//delete related shortcuts
					for(unsigned short i=0; i<tui_options.shortcuts_n; i++){
						if(tui_options.shortcuts[i].mode == mode_index_to_delete+1) delete_shortcut(i);
						else if(tui_options.shortcuts[i].mode > mode_index_to_delete+1) tui_options.shortcuts[i].mode--;
					}
				}else if(ui_index>=OPTIONS_TUI_BEGINNING_ELEMENTS+tui_options.modes_n){
					//delete shortcut
					unsigned short shortcut_index_to_delete = ui_index-(OPTIONS_TUI_BEGINNING_ELEMENTS+tui_options.modes_n+1);
					if(tui_options.shortcuts[shortcut_index_to_delete].type == SHORTCUT_TYPE_FULLSCREEN_QUIT){
						//make sure there's always at least one quit shortcut
						int quit_shortcuts_count = 0;
						for(unsigned short i=0; i<tui_options.shortcuts_n; i++){
							if(tui_options.shortcuts[i].type == SHORTCUT_TYPE_FULLSCREEN_QUIT){
								quit_shortcuts_count++;
							}
						}
						if(quit_shortcuts_count > 1){
							delete_shortcut(shortcut_index_to_delete);
							if(tui_options.shortcuts_n==0 || ui_index==OPTIONS_TUI_BEGINNING_ELEMENTS+tui_options.modes_n+1+tui_options.shortcuts_n) ui_index--; //if cursor was in last position
						}
					} else {
						delete_shortcut(shortcut_index_to_delete);
						if(tui_options.shortcuts_n==0 || ui_index==OPTIONS_TUI_BEGINNING_ELEMENTS+tui_options.modes_n+1+tui_options.shortcuts_n) ui_index--; //if cursor was in last position
					}
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
						log_debug("Adding new shortcut mode\n");
						ask_for_mode_name:
						char* mode_name = ask_for_mode_name(plane);
						if(mode_name!=NULL){
							if(!strcmp(mode_name,"default")){free(mode_name); goto ask_for_mode_name;}
							for(unsigned short i=0; i<tui_options.modes_n; i++){
								if(!strcmp(mode_name,tui_options.modes[i])){free(mode_name); goto ask_for_mode_name;}
							}
							tui_options.modes_n++;
							tui_options.modes = realloc(tui_options.modes, sizeof(char*)*(tui_options.modes_n+1));
							tui_options.modes[tui_options.modes_n-1] = mode_name;
							tui_options.modes[tui_options.modes_n] = NULL;
						}
						break;
					default:
						if(ui_index == OPTIONS_TUI_BEGINNING_ELEMENTS + tui_options.modes_n){
							log_debug("Adding new shortcut\n");
							struct shortcut shortcut = {};
							shortcut.value = 0;
							shortcut.string = NULL;
							shortcut.key = ask_for_key(plane);
							if(shortcut.key==NCKEY_ESC) break;
							add_shortcut_choose_action:
							char* shortcut_options[] = {"Tag file", "Untag file", "Tag/untag file", "Increase custom column value", "Decrease custom column value", "Reset custom column value", "Set custom column value", "External shell command on file", "Fullscreen: Next file", "Fullscreen: Previous file", "Fullscreen: Tag file", "Fullscreen: Options", "Fullscreen: External command", "Fullscreen: Quit", "Fullscreen: Choose shortcut mode", NULL};
							short choice = chooser(plane, shortcut_options, -1);
							if(choice==-1) break;
							log_debug("User chose action: '%s'\n", shortcut_options[choice]);
							shortcut.type = choice;
							switch(shortcut.type){
								case SHORTCUT_TYPE_TAG_FILE:
								case SHORTCUT_TYPE_UNTAG_FILE:
								case SHORTCUT_TYPE_TAG_UNTAG_FILE:
									shortcut.id = search_tag_tui();
									break;
								case SHORTCUT_TYPE_CUSTOM_COLUMN_INCREASE:
								case SHORTCUT_TYPE_CUSTOM_COLUMN_DECREASE:
								case SHORTCUT_TYPE_CUSTOM_COLUMN_RESET:
								case SHORTCUT_TYPE_CUSTOM_COLUMN_SET:
									if(custom_columns_n == 0){
										struct ncplane_options popup_opts = {
											.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
											.rows = 3, .cols = 50,
											.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED,
										};
										struct ncplane* popup = ncplane_create(plane, &popup_opts);
										ncplane_putstr_aligned(popup, 1, NCALIGN_CENTER, "No custom columns defined!");
										ncpile_render(plane);
										ncpile_rasterize(plane);
										notcurses_get(nc, NULL, NULL);
										ncplane_destroy(popup);
										goto add_shortcut_choose_action;
									}
																		shortcut.id = choose_custom_column(plane);
									if(shortcut.type == SHORTCUT_TYPE_CUSTOM_COLUMN_SET){
										char* value_str = ask_for_custom_column_value(plane);
										int value = atoi(value_str);
										if(value >= custom_columns[shortcut.id].lower_limit && value <= custom_columns[shortcut.id].upper_limit){
											shortcut.value = value;
										} else {
											shortcut.id = -1;
										}
										free(value_str);
									}
									log_debug("User chose custom column %d with name '%s'\n", (int)shortcut.id, custom_columns[shortcut.id].name);
									break;
								case SHORTCUT_TYPE_EXTERNAL_COMMAND:
									shortcut.string = ask_for_command(plane, screen_cols);
									if(shortcut.string==NULL) shortcut.id = -1;
									break;
								case SHORTCUT_TYPE_FULLSCREEN_NEXT:
								case SHORTCUT_TYPE_FULLSCREEN_PREV:
								case SHORTCUT_TYPE_FULLSCREEN_TAG:
								case SHORTCUT_TYPE_FULLSCREEN_OPTIONS:
								case SHORTCUT_TYPE_FULLSCREEN_COMMAND:
								case SHORTCUT_TYPE_FULLSCREEN_QUIT:
								case SHORTCUT_TYPE_FULLSCREEN_CHOOSE_MODE:
									break;
							}
							if(tui_options.modes_n>0){
								char** choices = malloc(sizeof(char*)*(tui_options.modes_n+2));
								choices[0] = "Default";
								for(unsigned m=0; m<tui_options.modes_n; m++) choices[m+1]=tui_options.modes[m];
								choices[tui_options.modes_n+1]=NULL;
								short choice = chooser(plane, choices, -1);
								free(choices);
								if(choice==-1){shortcut.id=-1; break;}
								shortcut.mode = choice;
							}
							if(shortcut.id!=-1){
								tui_options.shortcuts_n++;
								tui_options.shortcuts = realloc(tui_options.shortcuts, sizeof(struct shortcut)*tui_options.shortcuts_n);
								tui_options.shortcuts[tui_options.shortcuts_n-1] = shortcut;
							}
							break;
						}
				}
				break;
		}
		ui_elements = OPTIONS_TUI_BEGINNING_ELEMENTS + tui_options.modes_n + 1 + tui_options.shortcuts_n;
		ncplane_erase(plane);

		//mark current index
		if(ui_index<OPTIONS_TUI_BEGINNING_ELEMENTS-1) ncplane_putstr_yx(plane, ui_index, 0, "->");
		else if(ui_index<OPTIONS_TUI_BEGINNING_ELEMENTS+tui_options.modes_n) ncplane_putstr_yx(plane, 2+ui_index, 0, "->");
		else ncplane_putstr_yx(plane, 4+ui_index, 0, "->");
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

		//shortcut modes
		ncplane_putstr_yx(plane, OPTIONS_TUI_BEGINNING_ELEMENTS+1, 3, "Add new shortcut mode");
		for(unsigned short i=0; i<tui_options.modes_n; i++){
			ncplane_putstr_yx(plane, OPTIONS_TUI_BEGINNING_ELEMENTS+2+i, 4, tui_options.modes[i]);
		}

		//shortcuts
		ncplane_putstr_yx(plane, OPTIONS_TUI_BEGINNING_ELEMENTS+tui_options.modes_n+4, 3, "Add new shortcut");
		for(unsigned short i=0; i<tui_options.shortcuts_n; i++){
			unsigned short mode = tui_options.shortcuts[i].mode;
			if(mode==0) ncplane_printf_yx(plane, OPTIONS_TUI_BEGINNING_ELEMENTS+tui_options.modes_n+5+i, 3, "Default: %c --> ", tui_options.shortcuts[i].key);
			else ncplane_printf_yx(plane, OPTIONS_TUI_BEGINNING_ELEMENTS+tui_options.modes_n+5+i, 3, "%s: %c --> ", tui_options.modes[mode-1], tui_options.shortcuts[i].key);
			switch(tui_options.shortcuts[i].type){
				case SHORTCUT_TYPE_TAG_FILE:
					ncplane_putstr(plane, "Add tag to file: ");
					ncplane_putstr(plane, tag_name_from_id(tui_options.shortcuts[i].id));
					break;
				case SHORTCUT_TYPE_UNTAG_FILE:
					ncplane_putstr(plane, "Remove tag from file: ");
					ncplane_putstr(plane, tag_name_from_id(tui_options.shortcuts[i].id));
					break;
				case SHORTCUT_TYPE_TAG_UNTAG_FILE:
					ncplane_putstr(plane, "Add or remove tag to file: ");
					ncplane_putstr(plane, tag_name_from_id(tui_options.shortcuts[i].id));
					break;
				case SHORTCUT_TYPE_CUSTOM_COLUMN_INCREASE:
					ncplane_putstr(plane, "Increase value of custom column: ");
					ncplane_putstr(plane, custom_columns[tui_options.shortcuts[i].id].name);
					break;
				case SHORTCUT_TYPE_CUSTOM_COLUMN_DECREASE:
					ncplane_putstr(plane, "Decrease value of custom column: ");
					ncplane_putstr(plane, custom_columns[tui_options.shortcuts[i].id].name);
					break;
				case SHORTCUT_TYPE_CUSTOM_COLUMN_RESET:
					ncplane_putstr(plane, "Reset value of custom column: ");
					ncplane_putstr(plane, custom_columns[tui_options.shortcuts[i].id].name);
					break;
				case SHORTCUT_TYPE_CUSTOM_COLUMN_SET:
					ncplane_putstr(plane, "Set value of custom column: ");
					ncplane_printf(plane, "%s to %d", custom_columns[tui_options.shortcuts[i].id].name, tui_options.shortcuts[i].value);
					break;
				case SHORTCUT_TYPE_EXTERNAL_COMMAND:
					ncplane_putstr(plane, "External command: ");
					ncplane_putstr(plane, tui_options.shortcuts[i].string);
					break;
				case SHORTCUT_TYPE_FULLSCREEN_NEXT:
					ncplane_putstr(plane, "Fullscreen: Next file");
					break;
				case SHORTCUT_TYPE_FULLSCREEN_PREV:
					ncplane_putstr(plane, "Fullscreen: Previous file");
					break;
				case SHORTCUT_TYPE_FULLSCREEN_TAG:
					ncplane_putstr(plane, "Fullscreen: Tag file");
					break;
				case SHORTCUT_TYPE_FULLSCREEN_OPTIONS:
					ncplane_putstr(plane, "Fullscreen: Options");
					break;
				case SHORTCUT_TYPE_FULLSCREEN_COMMAND:
					ncplane_putstr(plane, "Fullscreen: External command");
					break;
				case SHORTCUT_TYPE_FULLSCREEN_QUIT:
					ncplane_putstr(plane, "Fullscreen: Quit");
					break;
				case SHORTCUT_TYPE_FULLSCREEN_CHOOSE_MODE:
					ncplane_putstr(plane, "Fullscreen: Choose shortcut mode");
					break;
			}
		}
		ncpile_render(plane);
		ncpile_rasterize(plane);
	}while((c=notcurses_get(nc, NULL, NULL))!='q');

	ncplane_destroy(plane);
	save_tui_options(INIT_DIRECTORY"/""tui_options");
}
