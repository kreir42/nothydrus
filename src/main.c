#include "nothydrus.h"

int main(int argc, char** argv){
	char* target_dir = ".";
	for(unsigned short i=1; i<argc; i++){
		if(!strcmp(argv[i], "init")){
			i++;
			if(i==(argc-1)){
				target_dir = argv[i];
			}else if(i!=argc){
				fprintf(stderr, "Error: init takes only one argument\n");
				return -1;
			}
			if(chdir(target_dir)){
				fprintf(stderr, "Error: could not chdir to directory %s. ", target_dir);
				perror(NULL);
				break;
			}
			init();
			break;
		}else{
			fprintf(stderr, "Error: unrecognized argument %s\n", argv[i]);
			return -1;
		}
	}
	return 0;
}
