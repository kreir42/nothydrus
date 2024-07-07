#include "nothydrus.h"
#include "tui.h"

#define OPTIONS_TUI_MIN_ELEMENTS 2

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
				if(ui_index<ui_elements) ui_index++;
				else ui_index = 0;
				break;
			case NCKEY_UP:
				if(ui_index>0) ui_index--;
				else ui_index = ui_elements;
				break;
			case 'd':
				if(ui_index>=OPTIONS_TUI_MIN_ELEMENTS){
					//delete shortcut
				}
				break;
			case '+':
				switch(ui_index){
					case 0: tui_options.search_limit++; break;
				}
				break;
			case '-':
				switch(ui_index){
					case 0: if(tui_options.search_limit>0) tui_options.search_limit--; break;
				}
				break;
			case NCKEY_ENTER:
				switch(ui_index){
					case 0:
						break;
					case 1:
						break;
				}
				break;
		}
		ui_elements = OPTIONS_TUI_MIN_ELEMENTS + tui_options.shortcuts_n;
		ncplane_erase(plane);

		//mark current index
		if(ui_index<OPTIONS_TUI_MIN_ELEMENTS-1) ncplane_putstr_yx(plane, ui_index, 0, "->");
		else ncplane_putstr_yx(plane, 2+ui_index, 0, "->");
		ncplane_printf_yx(plane, 0, 3, "Default search limit (0 for none): %lu", tui_options.search_limit);	//limit
		ncpile_render(plane);
		ncpile_rasterize(plane);
	}while((c=notcurses_get(nc, NULL, NULL))!='q');

	ncplane_destroy(plane);
}
