#include "nothydrus.h"
#include "tui.h"

void fullscreen_display(struct search* search){
	unsigned int screen_rows, screen_cols;
	notcurses_stddim_yx(nc, &screen_rows, &screen_cols);
	struct ncplane_options plane_options = {
		.rows = screen_rows, .cols = screen_cols,
	};
	struct ncplane* plane = ncpile_create(nc, &plane_options);
	ncplane_printf(plane, "1/%ld", search->output_ids.used);
	plane_options.y = 1;
	plane_options.rows = screen_rows-1;
	struct ncplane* display_plane = ncplane_create(plane, &plane_options);

	unsigned long i = 0;
	display_file(search->output_ids.data[0], 0, display_plane);
	ncpile_render(plane);
	ncpile_rasterize(plane);
	struct id_dynarr file_tags = new_id_dynarr(10);
	uint32_t c = NCKEY_RESIZE;
	bool quit = false;

	bool* active_modes = malloc(sizeof(bool)*(tui_options.modes_n));
	for(unsigned short j=0; j<tui_options.modes_n; j++) active_modes[j] = false;

	do{
		if(c != NCKEY_RESIZE){
			log_debug("Registered key %c\n", c);
			for(unsigned short j=0; j<tui_options.shortcuts_n; j++){
				if(c==tui_options.shortcuts[j].key && (tui_options.shortcuts[j].mode==0 || active_modes[tui_options.shortcuts[j].mode-1])){
					switch(tui_options.shortcuts[j].type){
						case SHORTCUT_TYPE_TAG_FILE:
							log_debug("Corresponds to SHORTCUT_TYPE_TAG_FILE\n");
							tag(search->output_ids.data[i], tui_options.shortcuts[j].id);
							break;
						case SHORTCUT_TYPE_UNTAG_FILE:
							log_debug("Corresponds to SHORTCUT_TYPE_UNTAG_FILE\n");
							untag(search->output_ids.data[i], tui_options.shortcuts[j].id);
							break;
						case SHORTCUT_TYPE_TAG_UNTAG_FILE:{
							log_debug("Corresponds to SHORTCUT_TYPE_TAG_UNTAG_FILE\n");
							get_file_tags(search->output_ids.data[i], &file_tags);
							bool has_tag = false;
							for(unsigned short k=0; k<file_tags.used; k++){
								if(file_tags.data[k] == tui_options.shortcuts[j].id){
									has_tag = true;
									break;
								}
							}
							if(has_tag) untag(search->output_ids.data[i], tui_options.shortcuts[j].id);
							else tag(search->output_ids.data[i], tui_options.shortcuts[j].id);
							break;}
						case SHORTCUT_TYPE_CUSTOM_COLUMN_INCREASE:{
							log_debug("Corresponds to SHORTCUT_TYPE_CUSTOM_COLUMN_INCREASE\n");
							get_file_columns(search->output_ids.data[i]);
							int value = sqlite3_column_int(get_file_columns_statement, NON_CUSTOM_FILE_COLUMNS+tui_options.shortcuts[j].id);
							if(value<custom_columns[tui_options.shortcuts[j].id].upper_limit) value++;
							set_custom_column_value(search->output_ids.data[i], tui_options.shortcuts[j].id, value);
							break;}
						case SHORTCUT_TYPE_CUSTOM_COLUMN_DECREASE:{
							log_debug("Corresponds to SHORTCUT_TYPE_CUSTOM_COLUMN_DECREASE\n");
							get_file_columns(search->output_ids.data[i]);
							int value = sqlite3_column_int(get_file_columns_statement, NON_CUSTOM_FILE_COLUMNS+tui_options.shortcuts[j].id);
							if(value>custom_columns[tui_options.shortcuts[j].id].lower_limit) value--;
							set_custom_column_value(search->output_ids.data[i], tui_options.shortcuts[j].id, value);
							break;}
						case SHORTCUT_TYPE_CUSTOM_COLUMN_RESET:
							log_debug("Corresponds to SHORTCUT_TYPE_CUSTOM_COLUMN_RESET\n");
							reset_custom_column_value(search->output_ids.data[i], tui_options.shortcuts[j].id);
							break;
						case SHORTCUT_TYPE_CUSTOM_COLUMN_SET:
							log_debug("Corresponds to SHORTCUT_TYPE_CUSTOM_COLUMN_SET\n");
							set_custom_column_value(search->output_ids.data[i], tui_options.shortcuts[j].id, tui_options.shortcuts[j].value);
							break;
						case SHORTCUT_TYPE_EXTERNAL_COMMAND:
							log_debug("Corresponds to SHORTCUT_TYPE_EXTERNAL_COMMAND\n");
							external_command_on_file(search->output_ids.data[i], tui_options.shortcuts[j].string);
							break;
						case SHORTCUT_TYPE_FULLSCREEN_NEXT:
							log_debug("Corresponds to SHORTCUT_TYPE_FULLSCREEN_NEXT\n");
							i++;
							if(i>=search->output_ids.used) i=0;
							reset_display_plane(display_plane);
							display_file(search->output_ids.data[i], 0, display_plane);
							{//prevent input from piling up
								struct timespec ts = {.tv_sec = 0, .tv_nsec = 10000000};
								while(notcurses_get(nc, &ts, NULL)){}
							}
							break;
						case SHORTCUT_TYPE_FULLSCREEN_PREV:
							log_debug("Corresponds to SHORTCUT_TYPE_FULLSCREEN_PREV\n");
							if(i==0) i = search->output_ids.used-1;
							else i--;
							reset_display_plane(display_plane);
							display_file(search->output_ids.data[i], 0, display_plane);
							{//prevent input from piling up
								struct timespec ts = {.tv_sec = 0, .tv_nsec = 10000000};
								while(notcurses_get(nc, &ts, NULL)){}
							}
							break;
						case SHORTCUT_TYPE_FULLSCREEN_TAG:
							log_debug("Corresponds to SHORTCUT_TYPE_FULLSCREEN_TAG\n");
							file_tag_tui(search->output_ids.data[i]);
							break;
						case SHORTCUT_TYPE_FULLSCREEN_OPTIONS:
							log_debug("Corresponds to SHORTCUT_TYPE_FULLSCREEN_OPTIONS\n");
							options_tui();
							break;
						case SHORTCUT_TYPE_FULLSCREEN_COMMAND:{
							log_debug("Corresponds to SHORTCUT_TYPE_FULLSCREEN_COMMAND\n");
							char* command = input_reader(plane, screen_rows-1, 0, 1, screen_cols-1, NULL, NULL, ":", false);
							if(command != NULL){
								external_command_on_file(search->output_ids.data[i], command);
								free(command);
							}
							break;}
						case SHORTCUT_TYPE_FULLSCREEN_QUIT:
							log_debug("Corresponds to SHORTCUT_TYPE_FULLSCREEN_QUIT\n");
							quit = true;
							break;
						case SHORTCUT_TYPE_FULLSCREEN_CHOOSE_MODE:{
							log_debug("Corresponds to SHORTCUT_TYPE_FULLSCREEN_CHOOSE_MODE\n");
							if(tui_options.modes_n>0){
								uint_least8_t initial_value = 0;
								for(unsigned short k=0; k<tui_options.modes_n; k++){
									if(active_modes[k]) initial_value |= (1<<k);
								}
								uint_least8_t result = multiple_chooser(plane, tui_options.modes, initial_value);
								for(unsigned short k=0; k<tui_options.modes_n-1; k++){
									if(result & (1<<k)) active_modes[k] = true;
									else active_modes[k] = false;
								}
							}
							break;}
					}
				}
			}
		}
		ncplane_erase(plane);
		//print file position
		ncplane_printf_yx(plane, 0, 0, "%ld/%ld", i+1, search->output_ids.used);
		//print filepath, filesize and resolution
		struct ncplane* child_plane = ncplane_userptr(display_plane);
		if(child_plane){
			struct ncvisual* visual = ncplane_userptr(child_plane);
			if(visual){
				ncvgeom geom;
				ncvisual_geom(nc, visual, NULL, &geom);
				ncplane_printf_aligned(plane, 0, NCALIGN_CENTER, "%s %f MB %dx%d", filepath_from_id(search->output_ids.data[i]), (double)filesize_from_id(search->output_ids.data[i])/1000000, geom.pixx, geom.pixy);
			} else {
				ncplane_printf_aligned(plane, 0, NCALIGN_CENTER, "%s %f MB", filepath_from_id(search->output_ids.data[i]), (double)filesize_from_id(search->output_ids.data[i])/1000000);
			}
		} else {
			ncplane_printf_aligned(plane, 0, NCALIGN_CENTER, "%s %f MB", filepath_from_id(search->output_ids.data[i]), (double)filesize_from_id(search->output_ids.data[i])/1000000);
		}
		//print file tags
		get_file_tags(search->output_ids.data[i], &file_tags);
		for(unsigned short j=0; j<file_tags.used; j++){
			char* tag_name = tag_name_from_id(file_tags.data[j]);
			ncplane_putstr_yx(plane, 1+j, 0, tag_name);
		}
		//print file custom columns
		get_file_columns(search->output_ids.data[i]);
		for(unsigned short j=0; j<custom_columns_n; j++){
			if(sqlite3_column_type(get_file_columns_statement, j+NON_CUSTOM_FILE_COLUMNS)==SQLITE_NULL) ncplane_printf_aligned(plane, 1+j, NCALIGN_RIGHT, "%s: NULL", custom_columns[j].name);
			else ncplane_printf_aligned(plane, 1+j, NCALIGN_RIGHT, "%s: %d", custom_columns[j].name, sqlite3_column_int(get_file_columns_statement, j+NON_CUSTOM_FILE_COLUMNS));
		}
		ncpile_render(plane);
		ncpile_rasterize(plane);
	}while(!quit && (c=notcurses_get(nc, NULL, NULL))!=0);
	log_debug("Exiting fullscreen_display mode\n");
	free(active_modes);
	free(file_tags.data);
	reset_display_plane(display_plane);
	ncpile_render(plane);		//to clear the screen TBD? way to do this without another render, rasterize?
	ncpile_rasterize(plane);
	ncplane_destroy(plane);
}
