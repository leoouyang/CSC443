#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <time.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#include "library.h"
using namespace std;


long now(){
    struct timeb t;
    ftime(&t);
    long ms = t.time * 1000 + t.millitm;
    return ms;
}

string addZeroToFront(int num)
{
    stringstream ss;

    // the number is converted to string
    ss << num;
    string result;
    ss >> result;

    // Append zero chars
    int str_length = result.length();
    for (int i = 0; i < 10 - str_length; i++)
        result = "0" + result;
    return result ;
}
// Build a column store from CSV file
// <colstore_name> should be a file directory to store the heap files

int main(int argc, char *argv[]){
	if (argc != 4){
		printf("Wrong number of Arguments. Usage: csv2colstore <csv_file> <colstore_name> <page_size>\n");
		exit(1);
	}
	
	ifstream csv_file;
	csv_file.open(argv[1]);
	if (!csv_file) {
		printf("Can't open csv file: %s\n", argv[1]);
		exit(1); 
	}

	char *colstore_dir = argv[2];

	if ((mkdir(colstore_dir, 0700)) == -1) {
        printf("Can't create Column Store Directory: %s\n", argv[2]);
		exit(1);
    }
	
	int page_size = atoi(argv[3]);
	
	//Reading all the Records from the csv_file first
	int total_records = 0;
	string str;
	int record_size;
	bool page_record_size_checked = false;

	//Store all rows of records
	std::vector<Record> *all_records = new std::vector<Record>();
	long start = now();
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
				printf("Record size is greater than datasize in page!!!\n");
				exit(1);
			}
		}
		all_records->push_back(record);
		total_records ++;
	}


	// Initialize ATTRIBUTE_NUM of Heap Files
	for (int i = 0;i < ATTRIBUTE_NUM;i++){
		std::stringstream ss;
		ss << colstore_dir << "/" << i;
		std::string filepath = ss.str();
		FILE *tempfile = fopen(filepath.c_str(),"w+");
		if (!tempfile){
			printf("Failed to Open/Create the Column HeapFile!!!\n");
			exit(1);
		}
		Heapfile *heapfile = (Heapfile*)malloc(sizeof(Heapfile));
		init_heapfile(heapfile, page_size, tempfile);


		Page *page = (Page*)malloc(sizeof(Page));
		init_fixed_len_page(page, page_size, ATTRIBUTE_SIZE * 2);
		int cap = fixed_len_page_capacity(page);
		//Store the records into the HeapFile now
		int cur_slot = 0;
		for (int j = 0; j < all_records->size();j++){
			Record curcol_record;
			string tuple_id = addZeroToFront(cur_slot);
			curcol_record.push_back(tuple_id.c_str());
			curcol_record.push_back(all_records->at(j).at(i));
			write_fixed_len_page(page, cur_slot, &curcol_record);
			cur_slot ++;

			if(cur_slot == cap){
				PageID pid = allocateCol_page(heapfile);
				write_page(page, heapfile, pid);
				cur_slot = 0;
				init_fixed_len_page(page, page_size, ATTRIBUTE_SIZE * 2);
			}
		}

		if(cur_slot != 0){
			PageID pid = allocateCol_page(heapfile);
			write_page(page, heapfile, pid);
			free(page->data);
			//init_fixed_len_page(page, page_size, ATTRIBUTE_SIZE * 2);
		}
		free(page);
		free(heapfile);
		fclose(tempfile);
	}


	long end = now();
	
	printf("NUMBER OF RECORDS: %d\n", total_records);
    printf("TIME: %ld milliseconds\n", end - start);

    csv_file.close();

}