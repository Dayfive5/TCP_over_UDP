#include <stdio.h>
#include <stdlib.h>


int main (int argc, char *argv[]) {

	char * seg = NULL;
	seg = "01234567";
	printf("%s\n", seg);

	seg = seg+1;
	printf("%s\n", seg);
	return 0;
}