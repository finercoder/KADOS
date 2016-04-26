/* @Copyright Fred Zhang Rose-Hulman Instuti
   A simple stricp that simplies your life in OS after Milestone 3.
   Simple compile it with gcc and run it.
   It will gives two exmaple of how to use it.
   It generates C code that construct local char array and fill it.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** args) {
	char* name;
	char* content;
	FILE* out = stdout;
	if (argc != 2) {
		name = args[1];
		content = args[2];
	} else if (argc != 3){
		name = args[1];
		content = args[2];
		out = fopen(args[3], "w");
	} else {
		fprintf(stderr, "%s <VarialbeName> <StringContent>\n", args[0]);
		fprintf(stderr, "%s <VarialbeName> <StringContent> <OutputFile>\n", args[0]);
		exit(1);
	}
	int len = strlen(content);
	fprintf(out, "char %s[%u];\n", name, len + 1);
	int i;
	for (i = 0; i < len ; i++) {
		fprintf(out, "%s[%d] = '%c';\n", name, i, content[i]);
	}
	fprintf(out, "%s[%d] = '\\0';\n", name, len);
	fclose(out);
}
