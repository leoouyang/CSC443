#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "library.h"

int fixed_len_sizeof(Record *record){
	return record->size() * strlen((*record)[0]);
}

void fixed_len_write(Record *record, void *buf){
	int record_size = fixed_len_sizeof(record);
	unsigned char serialized [record_size];
	int cur_index = 0;
	int attr_length = strlen((*record)[0]);
	
	for (int i = 0; i < record->size(); i++){
		memcpy(serialized + cur_index, (*record)[i], attr_length);
		cur_index += attr_length;
	}
	memcpy(buf, serialized, record_size);
}

void fixed_len_read(void *buf, int size, Record *record){
	char serialized[size];
	memcpy(serialized, buf, size);
	int cur_byte = 0;
	record->clear();
	
	while (cur_byte + ATTRIBUTE_SIZE <= size){
		char* attr = (char*)malloc(ATTRIBUTE_SIZE + 1);
		memcpy(attr, serialized + cur_byte, ATTRIBUTE_SIZE);
		attr[ATTRIBUTE_SIZE] = '\0';
		record->push_back(attr);
		cur_byte += ATTRIBUTE_SIZE;
	}
}

void init_fixed_len_page(Page *page, int page_size, int slot_size){
    page->page_size = page_size;
    page->slot_size = slot_size;
    page->data = malloc(page_size - sizeof(int));	//sizeof(int) is for the slot_size
	memset(page->data, '\0', page_size - sizeof(int));
}

int fixed_len_page_capacity(Page *page){
    return ((page->page_size - sizeof(int)) / page->slot_size); //sizeof(int) is for the slot_size
}																//no need to store page_size, same for every page in heap file

int fixed_len_page_freeslots(Page *page){
    int count = 0;
    int cap = fixed_len_page_capacity(page);
    for (int i = 0; i < cap; i++){
        if(((char*)page->data)[i * page->slot_size] == '\0' ){		//if the first character is '\0', empty slot
            count++;
        }
    }
    return count;
}

int add_fixed_len_page(Page *page, Record *r){
	int slot_num = -1;
	
	int cap = fixed_len_page_capacity(page);
	for (int i = 0; i < cap; i++){
        if( ((char*)page->data)[i * page->slot_size] == '\0' ){		//if the first character is '\0', empty slot
			void * cur_slot = (char*)page->data + (i * page->slot_size); //move the data pointer by slot size*slot position
            fixed_len_write(r, cur_slot);
			slot_num = i;
			break;
        }
    }
	return slot_num;
}

void write_fixed_len_page(Page *page, int slot, Record *r){
	void * cur_slot = (char*)page->data + (slot * page->slot_size);
    fixed_len_write(r, cur_slot);
}

void read_fixed_len_page(Page *page, int slot, Record *r){
	void * cur_slot = (char*)page->data + (slot * page->slot_size);
	fixed_len_read(cur_slot, page->slot_size, r);
}

//the structure of a serialized page stored in heap file is |slot_size|     data      |
void serialize_page(void *buf, Page *page){
	memcpy(buf, &page->slot_size, sizeof(int));
	memcpy((char *)buf + sizeof(int), page->data, page->page_size - sizeof(int));
}

void deserialize_page(void *buf, Page *page, int page_size){
	int slot_size;
	memcpy(&slot_size, buf, sizeof(int));
	init_fixed_len_page(page, page_size, slot_size);
	memcpy(page->data, (char *)buf + sizeof(int), page_size - sizeof(int));
}

void init_heapfile(Heapfile *heapfile, int page_size, FILE *file){
	heapfile->file_ptr = file;
	heapfile->page_size = page_size;
	
	//if this is a new heapfile, write the directory into the heap file
	//we can know whether this is a new one by checking the size of file
	fseek(file, 0, SEEK_END);
	if(ftell(file) == 0){
		//write the first directory page to file, slot size of directory is two integer, offset and freespace.
		fseek(file, 0, SEEK_SET);	//make sure file pointer is at start
		Page first_dir;
		init_fixed_len_page(&first_dir, page_size, sizeof(int) * 2);
		void * buf = malloc(page_size);
		serialize_page(buf, &first_dir);
		fwrite(buf, page_size, 1, file);
		// fwrite(&(first_dir->slot_size), sizeof(int), 1, file);
		// fwrite(first_dir->data, first_dir->page_size - sizeof(int), 1, file);
		fflush(file);
		
		free(buf);
		free(first_dir.data);
	}
}

PageID alloc_page(Heapfile *heapfile){
	FILE *file = heapfile->file_ptr;
	int page_size = heapfile->page_size;
	
	bool allocated = false;
	int cur_dir_page_offset = 0;
	int pageID = -1; //set to -1 so when we add 1 in the first loop, it become 0 as the pageID for first page in directory;
	while(!allocated){
		//set file pointer to the start of the directory
		fseek(file, cur_dir_page_offset, SEEK_SET);
		bool dir_dirty = false;
		
		//read the directory;
		void *read_dir_buf = malloc(page_size);
		fread(read_dir_buf, page_size, 1, file);
		Page cur_dir_page;
		deserialize_page(read_dir_buf, &cur_dir_page, page_size);
		free(read_dir_buf);
		
		//traverse directory entries in directory data for a free entry to hold new (page_offset, freespace)
		//the last directory entry is reserved for pointing to the next directory
		int cap = fixed_len_page_capacity(&cur_dir_page);
		for(int i = 0; i < (cap-1); i++){
			pageID++;
			int offset = ((int *)cur_dir_page.data)[i*2];
			int space = ((int *)cur_dir_page.data)[i*2 + 1];
			
			//if we find a free space, allocate new page
			//when we init page, empty space are init to '\0', which is 00000000 in binary
			if(offset == 0 and space == 0){
				fseek(file, 0, SEEK_END); // seek to end of file
				int new_offset = ftell(file); //offset if just current size of file
				Page new_page;
				init_fixed_len_page(&new_page, page_size, ATTRIBUTE_SIZE * ATTRIBUTE_NUM);  // ====================================Maybe need to modify for column store, what would be the slot size for that? Also, for previous functions, may not work if tuple id is stored as integer(fixed_len_page_freeslots, add_fixed_len_page), or if the size of tuple id string is not the same as other tuples(fixed_len_sizeof)==================================================
				
				//write new page to the end of heap file
				void *write_page_buf = malloc(page_size);
				serialize_page(write_page_buf, &new_page);
				fwrite(write_page_buf, page_size, 1, file);
				free(write_page_buf);
				
				//modify directory entries
				((int *)cur_dir_page.data)[i*2] = new_offset;
				((int *)cur_dir_page.data)[i*2 + 1] = fixed_len_page_capacity(&new_page);
				dir_dirty = true;
				allocated = true;
				
				free(new_page.data);
				break;
			}
		}
		
		int prev_dir_page_offset = cur_dir_page_offset;
		//if the new page is not yet allocated here, it mains there is no space in this directory
		if (!allocated){
			int offset = ((int *)cur_dir_page.data)[(cap-1)*2];
			int space = ((int *)cur_dir_page.data)[(cap-1)*2 + 1];
			//if next_directory pointer is empty, make new directory page, else check the next directory
			if (offset == 0 and space == 0){
				Page new_dir;
				fseek(file, 0, SEEK_END); // seek to end of file
				int new_dir_offset = ftell(file); //offset is just current size of file
				
				init_fixed_len_page(&new_dir, page_size, sizeof(int) * 2);
				void * buf = malloc(page_size);
				serialize_page(buf, &new_dir);
				fwrite(buf, page_size, 1, file);
				
				((int *)cur_dir_page.data)[(cap-1)*2] = new_dir_offset;
				cur_dir_page_offset = new_dir_offset;
				dir_dirty = true;
			}else{
				cur_dir_page_offset = offset;
			}
		}
		
		//write back directory if it is mordified
		if(dir_dirty){
			fseek(file, prev_dir_page_offset, SEEK_SET);
			void *write_dir_buf = malloc(page_size);
			serialize_page(write_dir_buf, &cur_dir_page);
			fwrite(write_dir_buf, page_size, 1, file);
			free(write_dir_buf);
		}
		
		free(cur_dir_page.data);
	}
	
	return pageID;
}

//if page_freespace >= 0, check whether it is changed and update directory
int find_page_offset(Heapfile *heapfile, PageID pid, int page_freespace){
	FILE *file = heapfile->file_ptr;
	int page_size = heapfile->page_size;
	
	bool finished = false;
	int pageID = -1;
	int cur_dir_page_offset = 0;
	int offset = -1;
	while(!finished){
		//set file pointer to the start of the directory
		fseek(file, cur_dir_page_offset, SEEK_SET);
		
		//read directory page
		void *read_dir_buf = malloc(page_size);
		fread(read_dir_buf, page_size, 1, file);
		Page cur_dir_page;
		deserialize_page(read_dir_buf, &cur_dir_page, page_size);
		free(read_dir_buf);
		
		//check for this pid in directory entry
		int cap = fixed_len_page_capacity(&cur_dir_page);
		for(int i = 0; i < (cap-1); i++){
			pageID++;
			//the rest of this directory is empty
			if (((int *)cur_dir_page.data)[i*2] == 0){
				break;
				finished = true;
			}
			if (pageID == pid){
				finished = true;
				offset = ((int *)cur_dir_page.data)[i*2];
				if (page_freespace >= 0){
					int freespace = ((int *)cur_dir_page.data)[i*2+1];
					if (freespace != page_freespace){
						((int *)cur_dir_page.data)[i*2+1] = page_freespace;
						//write directoy back with modified freespace
						fseek(file, cur_dir_page_offset, SEEK_SET);
						void *write_dir_buf = malloc(page_size);
						serialize_page(write_dir_buf, &cur_dir_page);
						fwrite(write_dir_buf, page_size, 1, file);
						free(write_dir_buf);
					}
				}
				break;
			}
		}
		
		if(!finished){
			int dir_offset = ((int *)cur_dir_page.data)[(cap-1)*2];
			int space = ((int *)cur_dir_page.data)[(cap-1)*2 + 1];
			//if next_directory pointer is empty, page with pid does not exist, else check the next directory
			if (dir_offset == 0 and space == 0){
				printf("Page with pid %d does not exist\n", pid);
				finished = true;
			}else{
				cur_dir_page_offset = dir_offset;
			}
			
		}
		free(cur_dir_page.data);
	}
	
	return offset;
}

void read_page(Heapfile *heapfile, PageID pid, Page *page){
	int page_size = heapfile->page_size;
	FILE *file = heapfile->file_ptr;
	int offset = find_page_offset(heapfile, pid, -1);
	
	if (offset > 0){
		//read the page and deserialize it into page object
		fseek(file, offset, SEEK_SET);
		void *read_page_buf = malloc(page_size);
		fread(read_page_buf, page_size, 1, file);
		deserialize_page(read_page_buf, page, page_size);
		free(read_page_buf);
	}
	
}

void write_page(Page *page, Heapfile *heapfile, PageID pid){
	int page_size = heapfile->page_size;
	FILE *file = heapfile->file_ptr;
	
	//pass in the freespace to check for possible directory entry change
	int freespace = fixed_len_page_freeslots(page);
	int offset = find_page_offset(heapfile, pid, freespace);
	
	if (offset > 0){
		//serialize the page and write it to the file
		fseek(file, offset, SEEK_SET);
		void *write_page_buf = malloc(page_size);
		serialize_page(write_page_buf, page);
		fwrite(write_page_buf, page_size, 1, file);
		free(write_page_buf);
	}
}

RecordIterator::RecordIterator(Heapfile *heapfile){
	this->heap = heapfile;
	int page_size = this->heap->page_size;
	FILE *file = this->heap->file_ptr;
	
	//read the first directory page and save it in cur_dir
	fseek(file, 0, SEEK_SET);
	void *read_dir_buf = malloc(page_size);
	fread(read_dir_buf, page_size, 1, file);
	this->cur_dir = (Page *)malloc(sizeof(Page));
	deserialize_page(read_dir_buf, this->cur_dir, page_size);
	free(read_dir_buf);
	this->dir_capacity = fixed_len_page_capacity(this->cur_dir);
	this->cur_page_dir_index = 0;
	int offset = ((int *)this->cur_dir->data)[0];
	
	//read the first data page of first directory and save it in cur_page
	fseek(file, offset, SEEK_SET);
	
	void *read_page_buf = malloc(page_size);
	fread(read_page_buf, page_size, 1, file);
	this->cur_page = (Page *)malloc(sizeof(Page));
	deserialize_page(read_page_buf, this->cur_page, page_size);
	free(read_page_buf);
	this->page_capacity = fixed_len_page_capacity(this->cur_page);
	this->cur_page_slot = 0;
	
	this->next_record.clear();
}

Record RecordIterator::next(){
	Record result;
	//next_record will be set to the next record is hasNext was called before this;
	if(!next_record.empty()){
		result = next_record;
		next_record.clear();
		return result;
	}else{
		this->hasNext();
		result = next_record;
		next_record.clear();
		return result;
	}
}

bool RecordIterator::hasNext(){
	//this happens when two hasNext() called in a row without next() in between
	if(!next_record.empty()){
		return true;
	}else{
		FILE *file = this->heap->file_ptr;
		int page_size = this->heap->page_size;
		
		bool finished = false;
		while(!finished){
			//check all pages in cur_dir
			bool more_pages = true;
			while(!finished & more_pages){
				//check every unread slot in the page, if not empty, we got what we want
				for (int i = (this->cur_page_slot); i < this->page_capacity; i++){
					this->cur_page_slot++;
					//if the first character is not '\0', record found
					if(((char*)this->cur_page->data)[i * this->cur_page->slot_size] != '\0' ){	
						void *cur_slot = (char*)this->cur_page->data + (i * this->cur_page->slot_size);
						fixed_len_read(cur_slot, this->cur_page->slot_size, &next_record);
						finished = true;
						break;
					}
				}
				//no more record in this page, go to next page
				if(!finished){
					// printf("changing page 1\n");
					this->cur_page_slot = 0;
					//cur_page's data is not needed anymore
					free(this->cur_page->data);
					//if cur_page_dir_index is capacity-2, reach the end of this directory(last directory entry is pointer to the next dir)
					// printf("cur_page_dir_index: %d\n", this->cur_page_dir_index);
					if(this->cur_page_dir_index == dir_capacity - 2){
						more_pages = false;
					}else{
						this->cur_page_dir_index += 1;
						int next_page_offset = ((int *)this->cur_dir->data)[(this->cur_page_dir_index) * 2];
						// printf("next_page_offset: %d\n", next_page_offset);
						//if true, we have no more unread page in heap file
						if (next_page_offset == 0){
							finished = true;
						}else{
							fseek(file, next_page_offset, SEEK_SET);
							void *read_page_buf = malloc(page_size);
							fread(read_page_buf, page_size, 1, file);
							//deserialize_page would reinitialized cur_page
							deserialize_page(read_page_buf, this->cur_page, page_size);
							free(read_page_buf);
						}
					}
				}
			}
			
			//no more record in the pages in this directory, go to next
			if (!finished){
				int next_dir_offset = ((int *)this->cur_dir->data)[(this->dir_capacity-1)*2];
				int space = ((int *)this->cur_dir->data)[(this->dir_capacity-1)*2 + 1];
				
				// printf("next_dir_offset: %d\n", next_dir_offset);
				//if next_directory pointer is empty, no more directory and record left, else check the next directory
				if (next_dir_offset == 0 and space == 0){
					finished = true;
				}else{
					//free the resource for old directory
					free(this->cur_dir->data);
					
					//read the next directory
					fseek(file, next_dir_offset, SEEK_SET);
					void *read_dir_buf = malloc(page_size);
					fread(read_dir_buf, page_size, 1, file);
					//deserialize_page would reinitialized cur_dir
					deserialize_page(read_dir_buf, this->cur_dir, page_size);
					free(read_dir_buf);
					
					//update informations of cur_page and read the first page into cur_page
					this->cur_page_dir_index = 0;
					int first_page_offset = ((int *)this->cur_dir->data)[0];
					// printf("first_page_offset: %d\n", first_page_offset);
					fseek(file, first_page_offset, SEEK_SET);
					void *read_first_page_buf = malloc(page_size);
					fread(read_first_page_buf, page_size, 1, file);
					//deserialize_page would reinitialized cur_page
					deserialize_page(read_first_page_buf, this->cur_page, page_size);
					free(read_first_page_buf);
				}
			}
		}
		
		if (!next_record.empty()){
			return true;
		}else{
			return false;
		}
	}
}

void free_record(Record *record){
	for(int i = 0; i < record->size(); i++){
		free((void *)record->at(i));
	}
}


PageID allocateCol_page(Heapfile *heapfile){
	FILE *file = heapfile->file_ptr;
	int page_size = heapfile->page_size;

	bool allocated = false;
	int cur_dir_page_offset = 0;
	int pageID = -1; //set to -1 so when we add 1 in the first loop, it become 0 as the pageID for first page in directory;
	while(!allocated){
		//set file pointer to the start of the directory
		fseek(file, cur_dir_page_offset, SEEK_SET);
		bool dir_dirty = false;

		//read the directory;
		void *read_dir_buf = malloc(page_size);
		fread(read_dir_buf, page_size, 1, file);
		Page cur_dir_page;
		deserialize_page(read_dir_buf, &cur_dir_page, page_size);
		free(read_dir_buf);

		//traverse directory entries in directory data for a free entry to hold new (page_offset, freespace)
		//the last directory entry is reserved for pointing to the next directory
		int cap = fixed_len_page_capacity(&cur_dir_page);
		for(int i = 0; i < (cap-1); i++){
			pageID++;
			int offset = ((int *)cur_dir_page.data)[i*2];
			int space = ((int *)cur_dir_page.data)[i*2 + 1];

			//if we find a free space, allocate new page
			//when we init page, empty space are init to '\0', which is 00000000 in binary
			if(offset == 0 and space == 0){
				fseek(file, 0, SEEK_END); // seek to end of file
				int new_offset = ftell(file); //offset if just current size of file
				Page new_page;
				init_fixed_len_page(&new_page, page_size, ATTRIBUTE_SIZE * 2);  // ====================================Maybe need to modify for column store, what would be the slot size for that? Also, for previous functions, may not work if tuple id is stored as integer(fixed_len_page_freeslots, add_fixed_len_page), or if the size of tuple id string is not the same as other tuples(fixed_len_sizeof)==================================================

				//write new page to the end of heap file
				void *write_page_buf = malloc(page_size);
				serialize_page(write_page_buf, &new_page);
				fwrite(write_page_buf, page_size, 1, file);
				free(write_page_buf);

				//modify directory entries
				((int *)cur_dir_page.data)[i*2] = new_offset;
				((int *)cur_dir_page.data)[i*2 + 1] = fixed_len_page_capacity(&new_page);
				dir_dirty = true;
				allocated = true;

				free(new_page.data);
				break;
			}
		}

		int prev_dir_page_offset = cur_dir_page_offset;
		//if the new page is not yet allocated here, it mains there is no space in this directory
		if (!allocated){
			int offset = ((int *)cur_dir_page.data)[(cap-1)*2];
			int space = ((int *)cur_dir_page.data)[(cap-1)*2 + 1];
			//if next_directory pointer is empty, make new directory page, else check the next directory
			if (offset == 0 and space == 0){
				Page new_dir;
				fseek(file, 0, SEEK_END); // seek to end of file
				int new_dir_offset = ftell(file); //offset is just current size of file

				init_fixed_len_page(&new_dir, page_size, sizeof(int) * 2);
				void * buf = malloc(page_size);
				serialize_page(buf, &new_dir);
				fwrite(buf, page_size, 1, file);

				((int *)cur_dir_page.data)[(cap-1)*2] = new_dir_offset;
				cur_dir_page_offset = new_dir_offset;
				dir_dirty = true;
			}else{
				cur_dir_page_offset = offset;
			}
		}

		//write back directory if it is mordified
		if(dir_dirty){
			fseek(file, prev_dir_page_offset, SEEK_SET);
			void *write_dir_buf = malloc(page_size);
			serialize_page(write_dir_buf, &cur_dir_page);
			fwrite(write_dir_buf, page_size, 1, file);
			free(write_dir_buf);
		}

		free(cur_dir_page.data);
	}

	return pageID;
}







