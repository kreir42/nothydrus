#include "nothydrus.h"
#include "tui.h"

short save_tui_options(char* name){
	log_debug("Saving tui options\n");
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
		case import_time:
			order = "import_time";
			break;
		case random_order:
			order = "random";
			break;
	}
	if(tui_options.search_descending){
		descending = "descending";
	}else{
		descending = "ascending";
	}
	fprintf(fp, "search_order_by = %s\n", order);
	fprintf(fp, "search_descending = %s\n", descending);
	fprintf(fp, "search_limit = %ld\n", tui_options.search_limit);

	fprintf(fp, "\n\nMODES\n");

	//write modes
	for(unsigned short i=0; i<tui_options.modes_n; i++){
		fprintf(fp, "%s\n", tui_options.modes[i]);
	}

	fprintf(fp, "\n\nSHORTCUTS\n");

	//write shortcuts
	for(unsigned short i=0; i<tui_options.shortcuts_n; i++){
		fprintf(fp, "KEY:%ld, TYPE:%d, MODE:%d, ID:%lld, VALUE:%d, STRING:%s\n", (long int)tui_options.shortcuts[i].key, (int)tui_options.shortcuts[i].type, (int)tui_options.shortcuts[i].mode, tui_options.shortcuts[i].id, tui_options.shortcuts[i].value, tui_options.shortcuts[i].string);
	}

	fclose(fp);
	return 0;
}

static void skip_whitespace(char** ptr){
	while(isspace(**ptr)){
		if(**ptr=='\n' || **ptr=='\0') break;
		*ptr+=1;
	}
}

static void skip_until_char(char** ptr, char target_char){
	while(**ptr!=target_char){
		if(**ptr=='\n' || **ptr=='\0') break;
		*ptr+=1;
	}
}

static void remove_trailing_spaces(char* str){
	unsigned short i = strlen(str);
	while(i>0 && isspace(str[i-1])!=0){
		i--;
		str[i] = '\0';
	}
}

void load_tui_options(char* name){
	log_debug("Loading tui options\n");
	if(access(name, F_OK)!=0){
		log_debug("Tried to load_tui_options but file doesn't exist\n");
		//file doesn't exist
		save_tui_options(name);
	}
	FILE* fp = fopen(name, "r");
	if(fp==NULL){
		fprintf(stderr, "load_tui_options:'%s' file could not be opened.\n", name);
		return;
	}
	char str[1000];
	char* current = str;

	//search_order_by
	fgets(str, 1000, fp);
	remove_trailing_spaces(str);
	skip_until_char(&current, '=');
	current++;
	skip_whitespace(&current);
	if(!strcmp(current, "none")) tui_options.search_order_by = none;
	else if(!strcmp(current, "size")) tui_options.search_order_by = size;
	else if(!strcmp(current, "import_time")) tui_options.search_order_by = import_time;
	else if(!strcmp(current, "random")) tui_options.search_order_by = random_order;
	else fprintf(stderr, "load_tui_options error: ------------------- %s", current);
	current=str;
	//search_descending
	fgets(str, 1000, fp);
	remove_trailing_spaces(str);
	skip_until_char(&current, '=');
	current++;
	skip_whitespace(&current);
	if(!strcmp(current, "descending")) tui_options.search_descending = 1;
	else if(!strcmp(current, "ascending")) tui_options.search_descending = 0;
	else fprintf(stderr, "load_tui_options error:");
	current=str;
	//search_limit
	fgets(str, 1000, fp);
	remove_trailing_spaces(str);
	skip_until_char(&current, '=');
	current++;
	skip_whitespace(&current);
	tui_options.search_limit = strtol(current, NULL, 10);
	current=str;

	//modes
	fgets(str, 1000, fp); //empty line
	fgets(str, 1000, fp); //empty line
	fgets(str, 1000, fp); //MODES
	unsigned short modes_n = 0;
	char** modes = malloc(200*sizeof(char*)); //TBD? remove max modes limit by having a proper dynarr?
	while(fgets(str, 1000, fp)!=NULL && str[0]!='\n'){
		remove_trailing_spaces(str);
		modes[modes_n] = malloc(sizeof(char)*strlen(str));
		strcpy(modes[modes_n], str);
		modes_n++;
	}
	modes = realloc(modes, sizeof(char*)*(modes_n+1));
	modes[modes_n] = NULL;
	tui_options.modes_n = modes_n;
	tui_options.modes = modes;

	//shortcuts
	unsigned short shortcuts_n = 0;
	struct shortcut* shortcuts = malloc(200*sizeof(struct shortcut)); //TBD? remove max shortcuts limit by having a proper dynarr?
	while(fgets(str, 1000, fp)!=NULL){
		long int key;
		int type;
		int mode;
		sqlite3_int64 id;
		int value;
		char string[2000]; string[0]='\0';
		if(sscanf(str, "KEY:%ld, TYPE:%d, MODE:%d, ID:%lld, VALUE:%d, STRING:%s", &key, &type, &mode, &id, &value, string)==6){
			shortcuts[shortcuts_n].key = key;
			shortcuts[shortcuts_n].type = type;
			shortcuts[shortcuts_n].mode = mode;
			shortcuts[shortcuts_n].id = id;
			shortcuts[shortcuts_n].value = value;
			shortcuts[shortcuts_n].string = malloc(sizeof(char)*strlen(string));
			strcpy(shortcuts[shortcuts_n].string, string);
			shortcuts_n++;
		}
	}
	shortcuts = realloc(shortcuts, sizeof(struct shortcut)*shortcuts_n);
	for(unsigned short i=0; i<tui_options.shortcuts_n; i++){
		if(tui_options.shortcuts[i].string) free(tui_options.shortcuts[i].string);
	}
	tui_options.shortcuts_n = shortcuts_n;
	free(tui_options.shortcuts);
	tui_options.shortcuts = shortcuts;

	fclose(fp);
	return;
}
