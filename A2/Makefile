# Makefile
CC = g++

all: csv2heapfile scan insert update delete select csv2colstore select2 select3 exp2 write_fixed_len_pages read_fixed_len_pages
 
library.o: library.cc library.h
	$(CC) -o $@ -c $<
 
csv2heapfile: csv2heapfile.cc library.o
	$(CC) -o $@ $< library.o
 
scan: scan.cc library.o
	$(CC) -o $@ $< library.o
 
insert: insert.cc library.o
	$(CC) -o $@ $< library.o
 
update: update.cc library.o
	$(CC) -o $@ $< library.o
 
delete: delete.cc library.o
	$(CC) -o $@ $< library.o
 
select: select.cc library.o
	$(CC) -o $@ $< library.o
 
csv2colstore: csv2colstore.cc library.o
	$(CC) -o $@ $< library.o
 
select2: select2.cc library.o
	$(CC) -o $@ $< library.o
 
select3: select3.cc library.o
	$(CC) -o $@ $< library.o

exp2: exp2.cc library.o
	$(CC) -o $@ $< library.o
	
write_fixed_len_pages: write_fixed_len_pages.cc library.o
	$(CC) -o $@ $< library.o

read_fixed_len_pages: read_fixed_len_pages.cc library.o
	$(CC) -o $@ $< library.o

clean:
	rm -f *.o csv2heapfile scan insert update delete select csv2colstore select2 select3 exp2 write_fixed_len_pages read_fixed_len_pages
