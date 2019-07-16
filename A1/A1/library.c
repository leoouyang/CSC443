#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <sys/timeb.h>

#include "library.h"

void random_array(char *array, long bytes){
	for (int i = 0; i < bytes; i++){
		array[i] = 'A' + (rand() % 26);
	}
}

int get_histogram(FILE *file_ptr, long hist[], int block_size, long *milliseconds, long *total_bytes_read){
	
	if (file_ptr){
		
		for (int i = 0; i < 26; i++){
			hist[i] = 0;
		}
		
		char buf[block_size];	
		int index;
		*total_bytes_read = 0;
		*milliseconds = 0;
		
		struct timeb start_time;
		struct timeb end_time;
		
		while(!feof(file_ptr)){
			
			ftime(&start_time);
			
			bzero(buf, block_size);
			fread(buf, 1, block_size, file_ptr);
			
			ftime(&end_time);
			*milliseconds += (end_time.time * 1000 + end_time.millitm) - (start_time.time * 1000 + start_time.millitm);

			for (long i = 0; i < block_size; i++){
				index = buf[i] - 'A'; 
				if (index >= 0 && index < 26){
					(*total_bytes_read)++;
					hist[index]++;
				}
			}
		
		}
		
		return 0;
	}else{
		return -1;
	}
}
