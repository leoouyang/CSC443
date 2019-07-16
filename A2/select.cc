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
    if (argc != 6){
        printf("Wrong number of Arguments. Usage: select <heapfile> <attribute_id> <start> <end> <page_size>\n");
        exit(1);
    }
    
    FILE *heap_fileptr;
    heap_fileptr = fopen(argv[1],"rb+");
    if (!heap_fileptr) {
        printf("Can't open heap file: %s\n", argv[1]);
        exit(1);
    }
    
    int attr_id = atoi(argv[2]);
    const char *start = argv[3];
    const char *end = argv[4];
    int page_size = atoi(argv[5]);

    Heapfile *heapfile = (Heapfile*)malloc(sizeof(Heapfile));
    init_heapfile(heapfile, page_size, heap_fileptr);
    
    RecordIterator iter(heapfile);
    
    int selected = 0;
    int total_records = 0;
    long start_time = now();
    long print_time = 0;
    
    while(iter.hasNext()){
        Record record = iter.next();
        char *attr = (char*)record.at(attr_id);
        
        if (strcmp(start, attr) <= 0 && strcmp(end, attr) >= 0) {
            long print_start = now();
            char *output_substring = new char[6];
            strncpy(output_substring, attr, 5);
            output_substring[5] = '\0';
            printf("%s\n", output_substring);
            print_time += now() - print_start;
            selected++;
        }
        total_records++;
        free_record(&record);
    }
    
    long end_time = now();
    
    printf("NUMBER OF TOTAL RECORDS: %d\n", total_records);
    printf("NUMBER OF SELECTED RECORDS: %d\n", selected);
    printf("TIME: %ld milliseconds\n", end_time - start_time - print_time);
    
    fclose(heap_fileptr);
    free(heapfile);
}
