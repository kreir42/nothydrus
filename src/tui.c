#include "nothydrus.h"
#include "tui.h"

struct notcurses* nc;
struct ncplane* search_planes[MAX_SEARCH_PLANES];

void start_tui(int_least8_t flags, void* data){
	if(!isatty(fileno(stdin))){
		freopen("/dev/tty", "r", stdin);	//reopen stdin if there was a pipe
	}

	struct notcurses_options opts = {
	};
	nc = notcurses_init(&opts, NULL);

	if(flags & START_TUI_DISPLAY){
		search_planes[0] = new_search_plane((struct search*)data);
	}else{
		search_planes[0] = new_search_plane(NULL);
	}
	search_plane(search_planes[0]);

	for(unsigned short i=0; i<MAX_SEARCH_PLANES; i++){
		if(search_planes[i]!=NULL) free_search_plane(search_planes[i]);
	}

	notcurses_drop_planes(nc);
	notcurses_stop(nc);
}
