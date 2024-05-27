#include "nothydrus.h"
#include "tui.h"

struct notcurses* nc;

void start_tui(int_least8_t flags){
//	setlocale(LC_ALL, "");
	struct notcurses_options opts = {
	};
	nc = notcurses_init(&opts, NULL);

	struct ncplane_options plane_options = {
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.rows = 20, .cols = 48,
		.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED,
	};
	struct ncplane* plane = ncplane_create(notcurses_stdplane(nc), &plane_options);
	display_file(1, DISPLAY_FILE_PIXEL, plane);

	ncpile_render(notcurses_stdplane(nc));
	ncpile_rasterize(notcurses_stdplane(nc));

	uint32_t c;
	while((c=notcurses_get(nc, NULL, NULL))!='q'){
		switch(c){
		}
	}

	notcurses_drop_planes(nc);
	notcurses_stop(nc);
}
