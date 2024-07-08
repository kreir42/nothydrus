#include "nothydrus.h"
#include "tui.h"

short save_tui_options(char* name){
	FILE* fp = fopen(name, "w");
	if(fp==NULL){
		fprintf(stderr, "save_tui_options:'%s' file could not be opened.\n", name);
		return 1;
	}

	//write options
	char* order, *descending;
	switch(tui_options.search_order_by){
		case none:
			order = "none";
			break;
		case size:
			order = "size";
			break;
		case random_order:
			order = "random";
			break;
	}
	if(tui_options.search_order_by!=none && tui_options.search_order_by!=random_order){
		if(tui_options.search_descending){
			descending = "descending";
		}else{
			descending = "ascending";
		}
	}else{
		descending = "";
	}
	fprintf(fp, "search_order_by = %s %s\n", order, descending);
	fprintf(fp, "search_limit = %ld\n", tui_options.search_limit);

	fprintf(fp, "\n\nSHORTCUTS\n");

	//write shortcuts
	for(unsigned short i=0; i<tui_options.shortcuts_n; i++){
		fprintf(fp, "KEY:%u, TYPE:%c, ID:%lld\n", tui_options.shortcuts[i].key, tui_options.shortcuts[i].type, tui_options.shortcuts[i].id);
	}

	fclose(fp);
	return 0;
}

short load_tui_options(char* name){
	FILE* fp = fopen(name, "r");
	if(fp==NULL){
		fprintf(stderr, "load_tui_options:'%s' file could not be opened.\n", name);
		return 1;
	}
	fclose(fp);
	return 0;
}
