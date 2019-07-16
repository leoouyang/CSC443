
#ifndef LIBRARY_H_
#define LIBRARY_H_

void random_array(char *array, long bytes);

int get_histogram(
    FILE *file_ptr, 
    long hist[], 
    int block_size, 
    long *milliseconds, 
    long *total_bytes_read);
	
#endif /* LIBRARY_H_ */
