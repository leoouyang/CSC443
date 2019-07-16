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
    if (argc != 4){
        printf("Wrong number of Arguments. Usage: delete <heapfile> <record_id> <page_size>\n");
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
    
    int page_size = atoi(argv[3]);
    
    Heapfile *heapfile = (Heapfile*)malloc(sizeof(Heapfile));
    init_heapfile(heapfile, page_size, heap_fileptr);

    Page *page = (Page*)malloc(sizeof(Page));
    read_page(heapfile, page_id, page);
    
//    Record r;
//    char *empty = new char[ATTRIBUTE_SIZE + 1];
//    empty[0] = '\0';
//    for(int i = 0; i < ATTRIBUTE_NUM; i++){
//        r.push_back(empty);
//    }
    
    void * cur_slot = (char*)page->data + (slot * page->slot_size);
    memset(cur_slot, '\0', page->slot_size);
    
    write_page(page, heapfile, page_id);
    
    free(page->data);
    free(page);
    free(heapfile);
    fclose(heap_fileptr);
}
