#include "nothydrus.h"
#include "tui.h"

struct ncplane* display_file(sqlite3_int64 id, int_least8_t flags, struct ncplane* plane){
	char* filepath = filepath_from_id(id);
	if(filepath!=NULL){
		return display_file_from_filepath(filepath, flags, plane);
	} else return NULL;
}

struct ncplane* display_file_from_filepath(char* filepath, int_least8_t flags, struct ncplane* plane){
	struct ncvisual* visual = ncvisual_from_file(filepath);
	struct ncvisual_options ncvisual_options = {
		.n = plane,
		.scaling = NCSCALE_SCALE_HIRES,
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.flags = NCVISUAL_OPTION_HORALIGNED|NCVISUAL_OPTION_VERALIGNED,
		.pxoffy = 0, .pxoffx = 0
	};
	if(flags & DISPLAY_FILE_PIXEL){
		ncvisual_options.blitter = NCBLIT_PIXEL;
		ncvisual_options.flags |= NCVISUAL_OPTION_CHILDPLANE;
	}else{
		ncvisual_options.blitter = NCBLIT_DEFAULT;
	}
	return ncvisual_blit(nc, visual, &ncvisual_options);
}