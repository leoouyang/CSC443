#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>

#include "library.h"


int main(int argc, char *argv[]){

	if (argc != 4){
		printf("Wrong number of arguments. To run the program: ./create_random_file <filename> <total bytes> <block size>\n");
		exit(-1);
	}

	char *filename = argv[1];
	long total_bytes = strtol(argv[2], NULL, 10);
	long block_size = strtol(argv[3], NULL, 10);
	
	if (total_bytes < 0 || block_size < 0){
		printf("total_bytes and block_size shouldn't be negative \n");
		exit(-1);
	}

	printf("Current Configuration: Filename = %s, Total Bytes = %ld, Block Size = %ld\n", filename, total_bytes, block_size);

	char *buf = (char *) malloc(block_size);
	FILE *fp = fopen(filename, "w");
	if (!fp) {
			perror("failed to open file");
			exit(-1);
	}

	struct timeb start_time;
	struct timeb end_time;
	long accumulator = 0;
	long write_size;
	
	for (long i = 0; i < total_bytes; i += block_size){
		
		if(i + block_size > total_bytes){
			write_size = total_bytes - i;
		}else{
			write_size = block_size;
		}
		
		random_array(buf, write_size);
		
		ftime(&start_time);
		
		fwrite(buf, 1, write_size, fp);
		fflush(fp);

		ftime(&end_time);

		accumulator += (end_time.time * 1000 + end_time.millitm) - (start_time.time * 1000 + start_time.millitm);
	}
	printf("It takes %ld milliseconds(%f seconds) to write the file\n", accumulator, accumulator/1000.0);
	printf("=======The write data rate is %f bytes/second=======\n", total_bytes/(accumulator/1000.0));\
	fclose(fp);
	free(buf);

	exit(0);
}

