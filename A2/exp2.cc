#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"

/**
 * Append charactor c to string s
 */
void append(char* s, char c) {
    int len = strlen(s);
    s[len] = c;
    s[len+1] = '\0';
}

int main(){
    char c;
    int r;
    
    // Construct a reandom record
    Record record;
    for(int i=0; i<100; i++){
        char *attr = (char *)malloc(ATTRIBUTE_SIZE+1);
        attr[0] = '\0';
        for(int j=0; j<10; j++){
            r = rand() % 26;
            c = 'a' + r;
            append(attr, c);
        }
        record.push_back(attr);
    }
    
    int size = fixed_len_sizeof(&record);
    printf("fixed_len_sizeof(record) returned %d, expected 100*10=1000\n", size);
    
    free_record(&record);
    
    //void *buf = malloc(size);
	
    //fixed_len_write(&record, buf);
	//Record record2;
    //fixed_len_read(buf, size, &record2);
    //printf("%s, %s\n", record[0], record2[0]);
    //printf("%s, %s\n", record[99], record2[99]);
}
