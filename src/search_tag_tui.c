#include "nothydrus.h"
#include "tui.h"

sqlite3_int64 search_tag_tui(){
	unsigned int screen_rows, screen_cols;
	notcurses_stddim_yx(nc, &screen_rows, &screen_cols);
	struct ncplane_options plane_options = {
		.y = 0, .x = 0,
		.rows = screen_rows, .cols = screen_cols,
	};
	struct ncplane* plane = ncpile_create(nc, &plane_options);

	sqlite3_int64 tag_id = -1;
	char* tag_search = NULL;
	struct id_dynarr search_results = new_id_dynarr(10);
	unsigned short ui_index = 0;
	uint32_t c = NCKEY_ENTER;
	ncplane_putstr_yx(plane, 0, 2, "Search here: ");
	do{
		switch(c){
			case NCKEY_DOWN:
				if(ui_index<search_results.used) ui_index++;
				else ui_index = 0;
				break;
			case NCKEY_UP:
				if(ui_index>0) ui_index--;
				else ui_index = search_results.used;
				break;
			case 'a':
				add_tag(tag_search);
				search_tags(&search_results, tag_search);
				break;
			case NCKEY_ENTER:
				if(ui_index==0){
					if(tag_search!=NULL) free(tag_search);
					tag_search = input_reader(plane, 0, 15, 1, screen_cols-16, NULL, NULL, NULL, false);
					if(tag_search != NULL) search_tags(&search_results, tag_search);
				}else{
					tag_id = search_results.data[ui_index-1];
					goto end_flag;
				}
				break;
		}
		ncplane_erase(plane);
		ncplane_printf_yx(plane, 0, 2, "Search here: %s", tag_search);
		if(search_results.used==0){
			ncplane_putstr_yx(plane, 2, 2, "No tag found, press 'a' to add search as new tag");
		}else{
			ncplane_putstr_yx(plane, 2, 2, "Search results:");
			for(unsigned short i=0; i<search_results.used; i++){
				ncplane_putstr_yx(plane, 3+i, 2, tag_name_from_id(search_results.data[i]));
			}
		}
		//mark current index
		if(ui_index==0) ncplane_putstr_yx(plane, 0, 0, "->");
		else ncplane_putstr_yx(plane, 2+ui_index, 0, "->");
		ncpile_render(plane);
		ncpile_rasterize(plane);
	}while((c=notcurses_get(nc, NULL, NULL))!='q');
	end_flag:
	ncplane_destroy(plane);
	if(tag_search!=NULL) free(tag_search);
	if(search_results.data!=NULL) free(search_results.data);
	return tag_id;
}
