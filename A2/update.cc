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
    if (argc != 6){
        printf("Wrong number of Arguments. Usage: update <heapfile> <record_id> <attribute_id> <new_value> <page_size>\n");
        exit(1);
    }
    
    FILE *heap_fileptr;
    heap_fileptr = fopen(argv[1],"rb+");
    if (!heap_fileptr) {
        printf("Can't open heap file: %s\n", argv[1]);
        exit(1);
    }
    
    PageID page_id;
    int slot;
    char *rec = strtok(argv[2], ",");
    if (rec != NULL){
        page_id = atoi(rec);
        rec = strtok(NULL, ",");
    } else {
        printf("Wrong format of <record_id>. Correct format: <page_id>,<slot>\n");
        exit(1);
    }
    if (rec != NULL){
        slot = atoi(rec);
    } else {
        printf("Wrong format of <record_id>. Correct format: <page_id>,<slot>\n");
        exit(1);
    }
    
    int attr_id = atoi(argv[3]);
    
    char *new_val = new char[ATTRIBUTE_SIZE + 1];
    strncpy(new_val, argv[4], ATTRIBUTE_SIZE);
    new_val[ATTRIBUTE_SIZE] = '\0';

    int page_size = atoi(argv[5]);
    
    Heapfile *heapfile = (Heapfile*)malloc(sizeof(Heapfile));
    init_heapfile(heapfile, page_size, heap_fileptr);
    
    Page *page = (Page*)malloc(sizeof(Page));
    read_page(heapfile, page_id, page);
    
    Record r;
    read_fixed_len_page(page, slot, &r);
    
    free(const_cast<char*>(r.at(attr_id)));
    r.at(attr_id) = new_val;

    write_fixed_len_page(page, slot, &r);
    write_page(page, heapfile, page_id);
    
    free(page->data);
    free(page);
    free(heapfile);
    free_record(&r);
    fclose(heap_fileptr);
}
