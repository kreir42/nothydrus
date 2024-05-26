#include "nothydrus.h"
#include "tui.h"

void display_file(sqlite3_int64 id, int_least8_t flags){
	char* filepath = filepath_from_id(id);
	if(filepath!=NULL){
		display_file_from_filepath(filepath, 0);
	}
}

void display_file_from_filepath(char* filepath, int_least8_t flags){
	struct ncvisual* visual = ncvisual_from_file(filepath);
	struct ncvisual_options ncvisual_options = {
		.n = notcurses_stdplane(nc),
		.scaling = NCSCALE_SCALE,
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.blitter = NCBLIT_DEFAULT,
		.flags = NCVISUAL_OPTION_HORALIGNED|NCVISUAL_OPTION_VERALIGNED | NCVISUAL_OPTION_CHILDPLANE,
	};
	struct ncplane* plane = ncvisual_blit(nc, visual, &ncvisual_options);
	ncpile_render(notcurses_stdplane(nc));
	ncpile_rasterize(notcurses_stdplane(nc));
}
