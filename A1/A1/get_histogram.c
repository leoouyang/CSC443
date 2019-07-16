#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "library.h"

int main(int argc, char *argv[]){
	
	if (argc != 3){
		printf("Wrong number of arguments. To run the program: ./get_histogram <filename> <blocksize>\n");
		exit(-1);
	}
	
	char *filename = argv[1];
	long block_size = strtol(argv[2], NULL, 10);
	
	if (block_size < 0){
		printf("block_size shouldn't be negative \n");
		exit(-1);
	}
	
	long hist[26];
	long milliseconds;
	long filelen;
	
	FILE *file_ptr = fopen(filename, "r");
	if (!file_ptr) {
			perror("failed to open file");
			exit(-1);
	}
	 
	int ret = get_histogram( file_ptr, 
							 hist, 
							 block_size,
							 &milliseconds,
							 &filelen);
	assert(! (ret < 0));

	for(int i=0; i < 26; i++) {
		printf("%c : %ld\n", 'A' + i, hist[i]);
	}
	
	printf("BLOCK SIZE %ld bytes\n", block_size);
	printf("TOTAL BYTES %ld bytes\n", filelen);
	printf("TIME %ld milliseconds\n", milliseconds);
	printf("READ DATA RATE %f bytes/second\n", filelen/(milliseconds/1000.0));

}
