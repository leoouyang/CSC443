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

long now(){
    struct timeb t;
    ftime(&t);
    long ms = t.time * 1000 + t.millitm;
    return ms;
}

int main(int argc, char *argv[]){
	if (argc != 4){
		printf("Wrong number of Arguments. Usage: insert <csv_file> <heapfile> <page_size>\n");
		exit(1);
	}
	
	ifstream csv_file;
	csv_file.open(argv[1]);
	if (!csv_file) {
		printf("Can't open csv file: %s\n", argv[1]);
		exit(1); 
	}
	
	FILE *heap_fileptr;
	heap_fileptr = fopen(argv[2],"rb+"); //if can't open with rb+, file do not exist, open with wb+ to write a new file
	if (!heap_fileptr) {
		heap_fileptr = fopen(argv[2],"wb+");
		if (!heap_fileptr){
			printf("Can't open heap file: %s\n", argv[2]);
			exit(1); 
		}
	}
	
	int page_size = atoi(argv[3]);
	
	Heapfile *heapfile = (Heapfile*)malloc(sizeof(Heapfile));
	init_heapfile(heapfile, page_size, heap_fileptr);
  
	Page *page = (Page*)malloc(sizeof(Page));
	init_fixed_len_page(page, page_size, ATTRIBUTE_SIZE * ATTRIBUTE_NUM);
	
	int total_records = 0;
	
	string str;
    bool page_record_size_checked = false;
	int record_size;
	int cur_slot = 0;
	int cap = fixed_len_page_capacity(page);
	while(getline(csv_file, str)){
		Record record;
		stringstream ss(str);
		string comma_sep;
		
		while(getline(ss, comma_sep, ',')){
            record.push_back(strdup(comma_sep.c_str()));
        }
		
		if(!page_record_size_checked){
			record_size = fixed_len_sizeof(&record);
			page_record_size_checked = true;
			if (record_size > (page_size - sizeof(int))){	
				printf("Record size is greater than datasize in page(datasize = pagesize - sizeof(int) for storing slot size)!!!\n");
				exit(1);
			}
		}
		
		write_fixed_len_page(page, cur_slot, &record);
		cur_slot++;
		
		if(cur_slot == cap){
			PageID pid = alloc_page(heapfile);
			write_page(page, heapfile, pid);
			total_records += cur_slot;
			cur_slot = 0;
			init_fixed_len_page(page, page_size, ATTRIBUTE_SIZE * ATTRIBUTE_NUM);
		}
        
        free_record(&record);
		
	}
	
	if(cur_slot != 0){
		PageID pid = alloc_page(heapfile);
		write_page(page, heapfile, pid);
		total_records += cur_slot;
		cur_slot = 0;
		free(page->data);
		init_fixed_len_page(page, page_size, ATTRIBUTE_SIZE * ATTRIBUTE_NUM);
	}
	
	printf("NUMBER OF RECORDS: %d\n", total_records);
	
	free(page->data);
	free(page);
	free(heapfile);
	
	csv_file.close();
	fclose(heap_fileptr);
}
