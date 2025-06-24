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
	char* tag_name;
	uint32_t c = NCKEY_RESIZE;
	do{
		switch(c){
			case NCKEY_RIGHT:
				i++;
				if(i>=search->output_ids.used) i=0;
				reset_display_plane(display_plane);
				display_file(search->output_ids.data[i], 0, display_plane);
				break;
			case NCKEY_LEFT:
				if(i==0) i = search->output_ids.used-1;
				else i--;
				reset_display_plane(display_plane);
				display_file(search->output_ids.data[i], 0, display_plane);
				break;
			case 't':
				file_tag_tui(search->output_ids.data[i]);
				break;
			case 'o':
				options_tui();
				break;
			case ':':
				char* command = input_reader(plane, screen_rows-1, 1, 1, screen_cols-2);
				external_command_on_file(search->output_ids.data[i], command);
				free(command);
				break;
			default:
				log_debug("Other input '%c'\n", c);
				for(unsigned short j=0; j<tui_options.shortcuts_n; j++){
					if(c==tui_options.shortcuts[j].key){
						switch(tui_options.shortcuts[j].type){
							case SHORTCUT_TYPE_TAG_FILE:
								log_debug("Corresponds to SHORTCUT_TYPE_TAG_FILE\n");
								tag(search->output_ids.data[i], tui_options.shortcuts[j].id);
								break;
							case SHORTCUT_TYPE_UNTAG_FILE:
								log_debug("Corresponds to SHORTCUT_TYPE_UNTAG_FILE\n");
								untag(search->output_ids.data[i], tui_options.shortcuts[j].id);
								break;
							case SHORTCUT_TYPE_TAG_UNTAG_FILE:
								log_debug("Corresponds to SHORTCUT_TYPE_TAG_UNTAG_FILE\n");
								//TBD
								break;
							case SHORTCUT_TYPE_CUSTOM_COLUMN_INCREASE:{
								log_debug("Corresponds to SHORTCUT_TYPE_CUSTOM_COLUMN_INCREASE\n");
								short custom_column_type = custom_columns[tui_options.shortcuts[j].id].type;
								switch(custom_column_type){
									case COLUMN_TYPE_TEXT:
										log_debug("Custom column is text type");
										break;
									case COLUMN_TYPE_INTEGER:{
										log_debug("Custom column is integer type");
										get_file_columns(search->output_ids.data[i]);
										int value = sqlite3_column_int(get_file_columns_statement, NON_CUSTOM_FILE_COLUMNS+tui_options.shortcuts[j].id);
										if(value<custom_columns[tui_options.shortcuts[j].id].upper_limit) value++;
										set_custom_column_value(search->output_ids.data[i], tui_options.shortcuts[j].id, &value);
										break;}
									case COLUMN_TYPE_REAL:{
										log_debug("Custom column is real type");
										get_file_columns(search->output_ids.data[i]);
										float value = sqlite3_column_double(get_file_columns_statement, NON_CUSTOM_FILE_COLUMNS+tui_options.shortcuts[j].id);
										if(value<custom_columns[tui_options.shortcuts[j].id].upper_limit) value++;
										set_custom_column_value(search->output_ids.data[i], tui_options.shortcuts[j].id, &value);
										break;}
								}
								break;}
							case SHORTCUT_TYPE_CUSTOM_COLUMN_DECREASE:{
								log_debug("Corresponds to SHORTCUT_TYPE_CUSTOM_COLUMN_DECREASE\n");
								short custom_column_type = custom_columns[tui_options.shortcuts[j].id].type;
								switch(custom_column_type){
									case COLUMN_TYPE_TEXT:
										log_debug("Custom column is text type");
										break;
									case COLUMN_TYPE_INTEGER:{
										log_debug("Custom column is integer type");
										get_file_columns(search->output_ids.data[i]);
										int value = sqlite3_column_int(get_file_columns_statement, NON_CUSTOM_FILE_COLUMNS+tui_options.shortcuts[j].id);
										if(value>custom_columns[tui_options.shortcuts[j].id].lower_limit) value--;
										set_custom_column_value(search->output_ids.data[i], tui_options.shortcuts[j].id, &value);
										break;}
									case COLUMN_TYPE_REAL:{
										log_debug("Custom column is real type");
										get_file_columns(search->output_ids.data[i]);
										float value = sqlite3_column_double(get_file_columns_statement, NON_CUSTOM_FILE_COLUMNS+tui_options.shortcuts[j].id);
										if(value>custom_columns[tui_options.shortcuts[j].id].lower_limit) value--;
										set_custom_column_value(search->output_ids.data[i], tui_options.shortcuts[j].id, &value);
										break;}
								}
								break;}
							case SHORTCUT_TYPE_CUSTOM_COLUMN_REMOVE:
								log_debug("Corresponds to SHORTCUT_TYPE_CUSTOM_COLUMN_REMOVE\n");
								switch(custom_columns[tui_options.shortcuts[j].id].type){
									case COLUMN_TYPE_TEXT:
										set_custom_column_value(search->output_ids.data[i], tui_options.shortcuts[j].id, "");
										break;
									case COLUMN_TYPE_INTEGER:{
										int reset_value = 0;
										set_custom_column_value(search->output_ids.data[i], tui_options.shortcuts[j].id, &reset_value);
										break;}
									case COLUMN_TYPE_REAL:{
										float reset_value = 0;
										set_custom_column_value(search->output_ids.data[i], tui_options.shortcuts[j].id, &reset_value);
										break;}
								}
								break;
							case SHORTCUT_TYPE_EXTERNAL_COMMAND:
								log_debug("Corresponds to SHORTCUT_TYPE_EXTERNAL_COMMAND\n");
								external_command_on_file(search->output_ids.data[i], tui_options.shortcuts[j].string);
								break;
						}
					}
				}
				break;
		}
		ncplane_erase(plane);
		//print file position
		ncplane_printf_yx(plane, 0, 0, "%ld/%ld", i+1, search->output_ids.used);
		//print filepath
		ncplane_printf_aligned(plane, 0, NCALIGN_CENTER, "%s %f MB", filepath_from_id(search->output_ids.data[i]), (double)filesize_from_id(search->output_ids.data[i])/1000000);
		//print file tags
		get_file_tags(search->output_ids.data[i], &file_tags);
		for(unsigned short j=0; j<file_tags.used; j++){
			tag_name = tag_name_from_id(file_tags.data[j]);
			ncplane_putstr_yx(plane, 1+j, 0, tag_name);
		}
		//print file custom columns
		for(unsigned short j=0; j<custom_columns_n; j++){
			get_file_columns(search->output_ids.data[i]);
			switch(custom_columns[j].type){
				case COLUMN_TYPE_TEXT:
					ncplane_printf_aligned(plane, 1+j, NCALIGN_RIGHT, "%s: %s", custom_columns[j].name, sqlite3_column_text(get_file_columns_statement, j+NON_CUSTOM_FILE_COLUMNS));
					break;
				case COLUMN_TYPE_INTEGER:
					ncplane_printf_aligned(plane, 1+j, NCALIGN_RIGHT, "%s: %d", custom_columns[j].name, sqlite3_column_int(get_file_columns_statement, j+NON_CUSTOM_FILE_COLUMNS));
					break;
				case COLUMN_TYPE_REAL:
					ncplane_printf_aligned(plane, 1+j, NCALIGN_RIGHT, "%s: %f", custom_columns[j].name, sqlite3_column_double(get_file_columns_statement, j+NON_CUSTOM_FILE_COLUMNS));
					break;
			}
		}
		ncpile_render(plane);
		ncpile_rasterize(plane);
	}while((c=notcurses_get(nc, NULL, NULL))!='q');
	free(file_tags.data);
	reset_display_plane(display_plane);
	ncpile_render(plane);		//to clear the screen TBD? way to do this without another render, rasterize?
	ncpile_rasterize(plane);
	ncplane_destroy(plane);
}
