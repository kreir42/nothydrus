#include "nothydrus.h"
#include "tui.h"

struct notcurses* nc;

void start_tui(int_least8_t flags){
//	setlocale(LC_ALL, "");
	struct notcurses_options opts = {
	};
	nc = notcurses_init(&opts, NULL);

	struct ncplane* plane = new_search_plane(NULL);
	if(flags & START_TUI_DISPLAY){
		//TBD
	}else{
		search_plane(plane);
	}

	free_search_plane(plane);
	notcurses_drop_planes(nc);
	notcurses_stop(nc);
}
