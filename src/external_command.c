#include "nothydrus.h"
#include "tui.h"

void external_command_on_file(sqlite3_int64 id, char* command){
	char* filepath = filepath_from_id(id);
	if(filepath!=NULL){
		char* system_string = malloc((strlen(command)+strlen(filepath)+30)*sizeof(char));
		system_string[0] = '\0';
		strcat(system_string, command);
		strcat(system_string, " ");
		strcat(system_string, filepath);
		strcat(system_string, " 1>/dev/null 2>&1 0>&1");
		int status = system(system_string);	//TBD check status
		free(system_string);
	}
}
