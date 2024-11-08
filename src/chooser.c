#include "nothydrus.h"
#include "tui.h"

short chooser(struct ncplane* parent_plane, char** options, short initial_value){
	unsigned short options_n = 0, length = 0, max_length = 0;
	while(options[options_n]!=NULL){
		length = strlen(options[options_n]);
		if(length>max_length) max_length = length;
		options_n++;
	}
	struct ncplane_options plane_options = {
		.x = NCALIGN_CENTER, .y = NCALIGN_CENTER,
		.rows = options_n+2, .cols = 4+max_length,
		.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED,
	};
	struct ncplane* plane = ncplane_create(parent_plane, &plane_options);
	//TBD draw a box

	for(unsigned short i=0; i<options_n; i++){
		ncplane_putstr_yx(plane, i+1, 4, options[i]);
	}

	uint32_t c = NCKEY_RESIZE;
	unsigned short option = 0;
	if(initial_value>=0) option = initial_value;
	do{
		ncplane_erase_region(plane, option+1, 1, options_n, 3);
		switch(c){
			case 'q':
			case 'Q':
				option = initial_value;
				//FALLTHROUGH
			case NCKEY_ENTER:
			case ' ':
				goto end_label;
			case NCKEY_DOWN:
				if(option<options_n-1) option++;
				else option = 0;
				break;
			case NCKEY_UP:
				if(option>0) option--;
				else option = options_n-1;
				break;
		}
		ncplane_putstr_yx(plane, option+1, 1, "->");
		ncpile_render(plane);
		ncpile_rasterize(plane);
	}while((c=notcurses_get(nc, NULL, NULL)));
	end_label:

	ncplane_destroy(plane);
	return option;
}

uint_least8_t multiple_chooser(struct ncplane* parent_plane, char** options, uint_least8_t initial_value){
	unsigned short options_n = 0, length = 0, max_length = 0;
	while(options[options_n]!=NULL){
		length = strlen(options[options_n]);
		if(length>max_length) max_length = length;
		options_n++;
	}
	struct ncplane_options plane_options = {
		.x = NCALIGN_CENTER, .y = NCALIGN_CENTER,
		.rows = options_n+2, .cols = 5+max_length,
		.flags = NCPLANE_OPTION_HORALIGNED | NCPLANE_OPTION_VERALIGNED,
	};
	struct ncplane* plane = ncplane_create(parent_plane, &plane_options);
	//TBD draw a box

	for(unsigned short i=0; i<options_n; i++){
		ncplane_putstr_yx(plane, i+1, 5, options[i]);
	}
	uint32_t c = NCKEY_RESIZE;
	uint_least8_t result = initial_value;
	unsigned short option=0;
	do{
		ncplane_erase_region(plane, option+1, 1, options_n, 4);
		switch(c){
			case 'q':
			case 'Q':
				goto end_label;
			case NCKEY_ENTER:
			case ' ':
				result ^= 1<<option; //binary XOR operator toggles the optionth flag
				break;
			case NCKEY_DOWN:
				if(option<options_n-1) option++;
				else option = 0;
				break;
			case NCKEY_UP:
				if(option>0) option--;
				else option = options_n-1;
				break;
		}
		ncplane_putstr_yx(plane, option+1, 1, "->");
		for(unsigned short i=0; i<options_n; i++){
			if(result&(1<<i)) ncplane_putchar_yx(plane, 1+i, 3, 'x');
		}
		ncpile_render(plane);
		ncpile_rasterize(plane);
	}while((c=notcurses_get(nc, NULL, NULL)));
	end_label:

	ncplane_destroy(plane);
	return result;
}
