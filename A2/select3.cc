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
    if (argc != 7){
        printf("Wrong number of Arguments. Usage: select3 <colstore_name> <attribute_id> <return_attribute_id> <start> <end> <page_size>\n");
        exit(1);
    }

    int attr_id = atoi(argv[2]);
    if (attr_id < 0 || attr_id >= ATTRIBUTE_NUM) {
        printf("Invalid Attribute ID: %d\n",attr_id);
        exit(1);
    }

    int return_attr_id = atoi(argv[3]);
    if (return_attr_id < 0 || return_attr_id >= ATTRIBUTE_NUM) {
        printf("Invalid Attribute ID: %d\n",return_attr_id);
        exit(1);
    }

    const char *columndir = argv[1];
    std::stringstream ss;
    ss << columndir << "/" << attr_id;
    std::string filepath = ss.str();
    FILE *heap_fileptr = fopen(filepath.c_str(),"rb+");
    if (!heap_fileptr){
        printf("Failed to Open Column HeapFile: %s\n",filepath.c_str());
        exit(1);
    }

    std::stringstream sd;
    sd << columndir << "/" << return_attr_id;
    std::string return_attr_filepath = sd.str();
    FILE *returnheap_fileptr = fopen(return_attr_filepath.c_str(),"rb+");
    if (!returnheap_fileptr){
        printf("Failed to Open Column HeapFile: %s\n",return_attr_filepath.c_str());
        exit(1);
    }


    const char *start = argv[4];
    const char *end = argv[5];
    int page_size = atoi(argv[6]);

    Heapfile *heapfile = (Heapfile*)malloc(sizeof(Heapfile));
    Heapfile *return_heapfile = (Heapfile*)malloc(sizeof(Heapfile));
    init_heapfile(heapfile, page_size, heap_fileptr);
    init_heapfile(return_heapfile, page_size, returnheap_fileptr);

    RecordIterator iter(heapfile);
    RecordIterator return_iter(return_heapfile);

    int selected = 0;
    int total_records = 0;
    long start_time = now();
    long print_time = 0;

    while(iter.hasNext() && return_iter.hasNext() ){
        Record record = iter.next();
        Record return_record = return_iter.next();
        char *attr = (char*)record.at(1);
        char *return_attr = (char*)return_record.at(1);
        // 1st attribute in Column Store is Tuple id
        // 2nd attribute in Column Store is the record
        //printf("%s, ", return_attr);

        if (strcmp(start, attr) <= 0 && strcmp(end, attr) >= 0) {
            long print_start = now();
            //printf("%s\n", return_attr);
            char *output_substring = new char[6];
            strncpy(output_substring, attr, 5);
            output_substring[5] = '\0';
            printf("%s\n", output_substring);
            print_time += now() - print_start;
            selected++;
        }
        total_records++;
        free_record(&record);
        free_record(&return_record);
    }

    long end_time = now();

    printf("NUMBER OF TOTAL RECORDS: %d\n", total_records);
    printf("NUMBER OF SELECTED RECORDS: %d\n", selected);
    printf("TIME: %ld milliseconds\n", end_time - start_time - print_time);

    fclose(heap_fileptr);
    free(heapfile);
    fclose(returnheap_fileptr);
    free(return_heapfile);
}
