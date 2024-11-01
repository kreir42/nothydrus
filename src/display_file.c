#include "nothydrus.h"
#include "tui.h"

struct ncplane* display_file(sqlite3_int64 id, int_least8_t flags, struct ncplane* plane){
	char* filepath = filepath_from_id(id);
	if(filepath!=NULL){
		return display_file_from_filepath(filepath, flags, plane);
	} else return NULL;
}

struct ncplane* display_file_from_filepath(char* filepath, int_least8_t flags, struct ncplane* plane){
	if(flags & (DISPLAY_FILE_MPV | DISPLAY_FILE_EXTERNAL)){
		char* system_string = malloc(50+strlen(filepath));
		system_string[0] = '\0';
		strcat(system_string, "mpv"" ");
		strcat(system_string, filepath);
		strcat(system_string, " 1>/dev/null 2>&1 0>&1");
		int status = system(system_string);	//TBD check status
		free(system_string);
		return NULL;
	}
	struct ncvisual* visual = ncvisual_from_file(filepath);
	struct ncvisual_options ncvisual_options = {
		.n = plane,
		.y = NCALIGN_CENTER, .x = NCALIGN_CENTER,
		.flags = NCVISUAL_OPTION_HORALIGNED|NCVISUAL_OPTION_VERALIGNED|NCVISUAL_OPTION_CHILDPLANE,
		.pxoffy = 0, .pxoffx = 0
	};
	if(flags & DISPLAY_FILE_FAST){
		ncvisual_options.scaling = NCSCALE_SCALE;
		ncvisual_options.blitter = NCBLIT_DEFAULT;
	}else{
		ncvisual_options.scaling = NCSCALE_SCALE_HIRES;
		ncvisual_options.blitter = NCBLIT_PIXEL;
	}
	struct ncplane* resultplane = ncvisual_blit(nc, visual, &ncvisual_options);
	ncplane_set_userptr(plane, resultplane);
	ncplane_set_userptr(resultplane, visual);
	return resultplane;
}

void reset_display_plane(struct ncplane* plane){
	struct ncplane* child_plane = ncplane_userptr(plane);
	if(child_plane!=NULL){
		ncvisual_destroy(ncplane_userptr(child_plane));
		ncplane_destroy(child_plane);
		ncplane_set_userptr(plane, NULL);
	}
	ncplane_erase(plane);
}
