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
		printf("Wrong number of Arguments. Usage: write_fixed_len_pages <csv file> <page file> <page size>\n");
		exit(1);
	}
	
	ifstream csv_file;
	csv_file.open(argv[1]);
	
	if (!csv_file) {
		printf("Can't open csv file: %s\n", argv[1]);
		exit(1); 
	}
	
	ofstream pagefile;
	pagefile.open(argv[2]);
	
	if (!pagefile) {
		printf("Can't open page file: %s\n", argv[2]);
		exit(1); 
	}
    
	int page_size = atoi(argv[3]);
    int SLOT_SIZE = ATTRIBUTE_SIZE * ATTRIBUTE_NUM;
	
    Page* page = (Page*)malloc(sizeof(Page));
    string str;
    init_fixed_len_page(page, page_size, SLOT_SIZE);
    
    int num_pages = 0;
    int num_records = 0;
    int total_records = 0;
    bool page_record_size_checked = false;
    int cap = fixed_len_page_capacity(page);
    int cur_slot = 0;
	
    long start = now();
    
    while(getline(csv_file, str)){
        Record r;
        stringstream ss(str);
        string comma_sep;
        
        while(getline(ss, comma_sep, ',')){
            r.push_back(strdup(comma_sep.c_str()));
        }
		if(!page_record_size_checked && fixed_len_sizeof(&r) > (page_size - sizeof(int))){ 
			printf("Record size is greater than datasize in page(datasize = pagesize - sizeof(int) for storing slot size)!!!\n");
			exit(1);
		}
		
		page_record_size_checked = true;
        
        write_fixed_len_page(page, cur_slot, &r);
        num_records++;
        cur_slot++;
        if(cap == num_records){
            void* buf = malloc(page->page_size);
            serialize_page(buf, page);
            pagefile.write((char*)buf, page->page_size);
            pagefile.flush();
            free(buf);

            free(page->data);

            num_pages++;
            total_records += num_records;
            num_records = 0;
            cur_slot = 0;
            init_fixed_len_page(page, page_size, SLOT_SIZE);
        }
        
        free_record(&r);
		
//        int slot_num = add_fixed_len_page(page, &r);
//        if (slot_num == -1){
//
//            void* buf = malloc(page->page_size);
//            serialize_page(buf, page);
//            // memcpy(buf, &page->slot_size, sizeof(int));
//            // memcpy((char *)buf + sizeof(int), page->data, page->page_size - sizeof(int));
//            pagefile.write((char*)buf, page->page_size);
//            pagefile.flush();
//            free(buf);
//
//            free(page->data);
//
//            num_pages++;
//            total_records += num_records;
//            num_records = 0;
//            init_fixed_len_page(page, page_size, SLOT_SIZE);
//            slot_num = add_fixed_len_page(page, &r);
//        }
//        num_records++;
    }
    
    if(num_records != 0){
		
        void* buf = malloc(page->page_size);
		serialize_page(buf, page);
		// memcpy(buf, &page->slot_size, sizeof(int));
		// memcpy((char *)buf + sizeof(int), page->data, page->page_size - sizeof(int));
		pagefile.write((char*)buf, page->page_size);
		pagefile.flush();
		free(buf);
		
		free(page->data);
		
        num_pages++;
        total_records += num_records;
    }
    
    long end = now();
    
    printf("NUMBER OF RECORDS: %d\n", total_records);
    printf("NUMBER OF PAGES: %d\n", num_pages);
    printf("TIME: %ld milliseconds\n", end - start);
    
    csv_file.close();
    pagefile.close();
	
	// ifstream pagefilein;
	// pagefilein.open(argv[2]);
	// void *buf = malloc(page_size);
	// pagefilein.read((char *)buf, page_size);
	// Page test_page;
	// deserialize_page(buf, &test_page, page_size);
	// printf("%d\n", test_page.slot_size);
	// printf("%d\n", test_page.page_size);
	// printf("%d\n", ((int *)(test_page.data))[0]);
	// char *data = static_cast<char*>(test_page.data);
	// printf("%c\n", data[page_size - sizeof(int)-1]);
	
	// ifstream pagefilein;
	// pagefilein.open(argv[2]);
	// int slot;
	// for (int i = 0; i < num_pages; i++){
		// pagefilein.seekg(i*page_size);
		// pagefilein.read((char *)&slot, sizeof(int));
		// printf("%d: %d\n", i, slot);
	// }
	// pagefilein.close();
	
    return 0;
}
