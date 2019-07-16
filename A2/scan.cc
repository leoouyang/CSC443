#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <string>
#include <time.h>
#include <sys/timeb.h>

#include "library.h"
using namespace std;

int main(int argc, char *argv[]){
	if (argc != 3){
		printf("Wrong number of Arguments. Usage: scan <heapfile> <page_size>\n");
		exit(1);
	}
	
	FILE *heap_fileptr;
	heap_fileptr = fopen(argv[1],"rb+");
	if (!heap_fileptr) {
		printf("Can't open heap file: %s\n", argv[1]);
		exit(1); 
	}
	
	int page_size = atoi(argv[2]);
	
	Heapfile *heapfile = (Heapfile*)malloc(sizeof(Heapfile));
	init_heapfile(heapfile, page_size, heap_fileptr);

	RecordIterator iter(heapfile);

	int total_record = 0;
	while (iter.hasNext()){
		Record record = iter.next();
		for(int i = 0; i < ATTRIBUTE_NUM - 1; i++){
			printf("%s, ", record[i]);
			free((void *)record[i]);
		}
		printf("%s\n\n", record[ATTRIBUTE_NUM - 1]);
		free((void *)record[ATTRIBUTE_NUM - 1]);
		total_record++;
	}
	printf("NUMBER OF RECORDS PRINTED: %d\n", total_record);
	
	fclose(heap_fileptr);
	free(heapfile);
}