#include "nothydrus.h"
#include "tui.h"

struct notcurses* nc;

void start_tui(int_least8_t flags){
//	setlocale(LC_ALL, "");
	struct notcurses_options opts = {
	};
	nc = notcurses_init(&opts, NULL);

	display_file(1, 0);

	uint32_t c;
	while((c=notcurses_get(nc, NULL, NULL))!='q'){
		switch(c){
		}
	}

	notcurses_drop_planes(nc);
	notcurses_stop(nc);
}
