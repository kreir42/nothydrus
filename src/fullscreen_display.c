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
	char* tag_fullname;
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
			case 'e':
				display_file(search->output_ids.data[i], DISPLAY_FILE_EXTERNAL, display_plane);
				break;
			case 'm':
				display_file(search->output_ids.data[i], DISPLAY_FILE_MPV, display_plane);
				break;
			case 'T':
				file_tag_tui(search->output_ids.data[i]);
				break;
		}
		ncplane_erase(plane);
		ncplane_printf_yx(plane, 0, 0, "%ld/%ld", i+1, search->output_ids.used);
		ncplane_printf_aligned(plane, 0, NCALIGN_CENTER, "%s %f MB", filepath_from_id(search->output_ids.data[i]), (double)filesize_from_id(search->output_ids.data[i])/1000000);
		get_file_tags(search->output_ids.data[i], &file_tags);
		for(unsigned short j=0; j<file_tags.used; j++){
			tag_fullname = tag_fullname_from_id(file_tags.data[j]);
			ncplane_putstr_yx(plane, 1+j, 0, tag_fullname);
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
