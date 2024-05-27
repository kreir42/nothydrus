#include "nothydrus.h"
#include "tui.h"

struct notcurses* nc;

void start_tui(int_least8_t flags){
//	setlocale(LC_ALL, "");
	struct notcurses_options opts = {
	};
	nc = notcurses_init(&opts, NULL);

	struct ncplane* new_search_plane = search_plane(NULL);

	ncpile_render(new_search_plane);
	ncpile_rasterize(new_search_plane);
	uint32_t c;
	while((c=notcurses_get(nc, NULL, NULL))!='q'){
		switch(c){
		}
		ncpile_render(new_search_plane);
		ncpile_rasterize(new_search_plane);
	}

	free_search_plane(new_search_plane);
	notcurses_drop_planes(nc);
	notcurses_stop(nc);
}
