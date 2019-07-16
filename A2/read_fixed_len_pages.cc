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
    if (argc != 3){
        printf("Wrong number of Arguments. Usage: read_fixed_len_pages <page file> <page size>\n");
        exit(1);
    }
    
    FILE *pagefile = fopen(argv[1], "r+");
    if (!pagefile) {
        printf("Can't open pagefile: %s\n", argv[1]);
        exit(1);
    }
    
    int page_size = atoi(argv[2]);
    int SLOT_SIZE = ATTRIBUTE_SIZE * 100;
    
    Page *page = (Page*) malloc(sizeof(Page));
    init_fixed_len_page(page, page_size, SLOT_SIZE);
    
    int cap = fixed_len_page_capacity(page);
    Record r;
    
    int num_pages = 0;
    int num_records = 0;
    void *buf = malloc(page_size);
    
    long print_time = 0;
    long start = now();
    
    while(!feof(pagefile)){
        int res = fread(buf, page_size, 1, pagefile);
        if(res == 0){
            break;
        }
		
        deserialize_page(buf, page, page_size);
        for(int i = 0; i < cap; i++){
            read_fixed_len_page(page, i, &r);
            
            long print_start = now();

            if(!strcmp(r.at(0), "")){
                break;
            }
            num_records++;
            for(int j = 1; j < ATTRIBUTE_NUM - 1; j++){
                printf("%s,", r.at(j));
                
            }
			printf("%s", r.at(ATTRIBUTE_NUM - 1));
            printf("\n");
            
            long print_end = now();
            print_time += print_end - print_start;
        }
        num_pages++;
    }
    
    long end = now();

    fclose(pagefile);
    free_record(&r);
    
    printf("\nNUMBER OF RECORDS: %d\n", num_records);
    printf("NUMBER OF PAGES: %d\n", num_pages);
    printf("TIME: %ld milliseconds\n", end - start - print_time);
    
    return 0;
    
}
