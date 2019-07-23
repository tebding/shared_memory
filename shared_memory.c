//main.c

#include <string.h>
#include <unistd.h> //probably not necessary?
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h> //enables mmap()
#include <fcntl.h> //for O_* constants
#include <sys/stat.h> //for mode constants

#define BUFF_SIZE 1024 //can increase if >1K values needed
#define SHM_SIZE 1021 //prime number: reduces hashing collisions.
/* CAN CHANGE SHM_SIZE TO BEST FIT SPEED/SIZE REQUIREMENTS */

typedef struct item {
    int key;
    char val[BUFF_SIZE];
} item;


//simple modular hash to start off with
int hash (int index) {
    return (index % SHM_SIZE);
}

/*
    update_shm: writes input file's k/v pair(s) to shared memory
PARAMs:
    fp: file pointer opened in main()
    fd: file descriptor to reference "shm" file, as determined in main()
RETURN: 1 for error, 0 for success
*/
int update_shm(FILE *fp, int fd) {
    item pairs[SHM_SIZE]; //array of entries: CAN SHORTEN IF LONGER THAN NEEDED
    char* buffer; //for reading from file
    char* tok;
    const char delim[2] = " ";
    int i, index, k, j = 0; //for handling loops + table

    //read input file into buffer, then tokenize buffer. repeat until EOF
    while (fgets(buffer, BUFF_SIZE, fp) != NULL) { //read input line -> buffer
        tok = strtok(buffer, delim); //tokenize the line
        while (tok != NULL) {
            //iterates through tokens for each line.
            pairs[i].key = atoi(tok); //first item is key
            tok = strtok(NULL, delim); //tokenize to 2nd item
            strcpy(pairs[i].val, tok); //second item is value
            i++;
        }
        //repeats this loop for each line from the input file.
    }

    //map the "shm" file to an array of items named 'table'
    item *table = mmap(NULL, SHM_SIZE, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
    //check for error in mapping
    if (table == MAP_FAILED) {
        perror("failed to mmap");
        return 1;
    }

    //hash keys, and copy values into shm indices
    while (j < i) {
        //find which index to write to for current index of 'pairs'
        index = hash(pairs[j].key);
        //linearly probe to resolve collisions
        //will overwrite stored values with the same key
        while ((table[index].key != NULL || table[index].key != pairs[j].key)
                && (k < SHM_SIZE)) {
            index++;
            k++; //tracks number of increments to prevent infinite loop
        }
        if (k >= SHM_SIZE) { //table is full! no more room to insert
            fprintf(stderr, "cannot insert: table is full!");
            return 1;
        }
        //by including the key in the table, can easily check collisions.
        table[index].key = pairs[j].key; //redundant if overwriting...

        //note: I'd use strncpy() but I can't identify any case where it's needed
        //and the required extra call to strlen() or sizeof() would be slower.
        strcpy(table[index].val, pairs[j].val);

        //to handle next key/val pair write
        j++;
    }

    //unmap the table to clean up
    if(munmap(table, SHM_SIZE) == -1) {
        perror("unmap failure");
        return 1;
    }

    //return success!
    return 0;
}


/*
    delete_shm: removes input file's k/v pairs from shared memory
PARAMS:
    fp: file pointer opened in main()
    fd: file descriptor to reference "shm" file, as determined in main()
RETURN: 1 for error, 0 for success
*/
int delete_shm(FILE *fp, int fd) {
    int keys[SHM_SIZE]; //int array is cheaper than item array + still suffices
    int i, index, k, j = 0;
    char* buffer; //for file reading
    char* tok;
    const char delim[2] = " ";

    //read the input file into buffer, then tokenize the input. repeat until EOF
    while (fgets(buffer, BUFF_SIZE, fp) != NULL) { //read input line -> buffer
        tok = strtok(buffer, delim); //tokenize the line
        while (tok != NULL) {
            //iterates through tokens for each line.
            keys[i] = atoi(tok); //first item is key (converted to int)
            tok = strtok(NULL, delim); //tokenize to 2nd item
            //doesn't add 2nd one to array because only the keys are needed
            i++;
        }
        //repeats this loop for each line from the input file.
    }

    //requisite keys are now known. next: map the shared memory
    item *table = mmap(NULL, SHM_SIZE, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
    //check for error with mapping
    if (table == MAP_FAILED) {
        perror("failed to mmap");
        return 1;
    }

    //shared memory is now mapped. next: iterate through keys[],
    //and delete the associated vals in shared memory
    while (j < i) {
        index = hash(keys[j]);
        //find items that were collisions
        while ((table[index].key != keys[j]) && (k < SHM_SIZE)) {
            if (table[index].key == NULL) {
                break; //error handled in next 'if' statement
            }
            index++;
            k++;
        }
        if (k >= SHM_SIZE || table[index].key == NULL) {
            fprintf(stderr, "ERROR: key \"%d\" not found.", keys[j]);
        }

        //actual deletion process
        for (k = 0; k < BUFF_SIZE; k++) { //k can be re-used to save time
            //clears every index of the string for security.
            if (table[index].val[k] != '\0') {
                table[index].val[k] = '\0';
                k++;
            }
            else { //at end of string. skip the trailing emptiness to save cycles
                break;
            }
            /*
            IF SPEED > SECURITY, use this INSTEAD OF above 'if-else' statement!
            table[index].val = '\0';
            */
        }
        table[index].key = NULL; //clears the key

        //handle the next key in next loop iteration
        j++;
        k = 0;
    }

    //clean up
    if (munmap(table, SHM_SIZE) == -1) {
        perror("failed to unmap");
        return 1;
    }

    //return success!
    return 0;
}


/*
    print_shm: prints values associated with keys in input file from shared memory
PARAMS:
    fp: file pointer opened in main()
    fd: file descriptor to reference "shm" file, as determined in main()
RETURN: 1 for failure, 0 for success
*/
int print_shm(FILE *fp, int fd) {
    int keys[SHM_SIZE]; //only keys are needed to ID index to print
    int i, index, k, j = 0; //for looping
    char* buffer; //for reading input file
    char* tok;
    const char delim[2] = " ";

    //read the input file into buffer, then tokenize the input. repeat until EOF
    while (fgets(buffer, BUFF_SIZE, fp) != NULL) { //read input line -> buffer
        tok = strtok(buffer, delim); //tokenize the line
        while (tok != NULL) {
            //iterates through tokens for each line.
            keys[i] = atoi(tok); //first item is key (converted to int)
            tok = strtok(NULL, delim); //tokenize to 2nd item
            //doesn't add 2nd one to array because only the keys are needed
            i++;
        }
        //repeats this loop for each line from the input file.
    }

    //requisite keys are now known. next: map the shared memory
    item *table = mmap(NULL, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    //check for error with mapping
    if (table == MAP_FAILED) {
        perror("failed to mmap");
        return 1;
    }

    //iterate through keys[] and print the associated vals
    while (j < i) {
        index = hash(keys[j]);
        //find items that were collisions
        while ((table[index].key != keys[j]) && (k < SHM_SIZE)) {
            if (table[index].key == NULL) {
                break;
            }
            index++;
            k++; //track iterations to prevent infinite looping
        }
        //if key doesn't exist
        if (k >= SHM_SIZE || table[index].key == NULL) {
            fprintf(stderr, "ERROR: key \"%d\" not found.", keys[j]);
        }

        //executes the printing for the current key
        printf("key %d has value: %s", table[index].key, table[index].val);

        //handle the next key in next loop iteration
        j++;
    }
    return 0;
}


int main(int argc, char const *argv[]) {
    /*
        I opted against combining the shm_open() calls in order to
        eliminate the otherwise-inevitable calls to ftruncate() that
        are entirely unnecessary except for when O_CREAT is invoked.
    */
    int fd = shm_open("shm", O_RDWR, 0666);
    if (fd == -1) { //memory space does not yet exist
        fd = shm_open("shm", O_CREAT, 0666);
        if (fd == -1) { //failed to create memory space
            perror("failed to open/create:");
            return 1;
        }
        int size = SHM_SIZE*sizeof(item);
        //set the space to the desired size
        if (ftruncate(fd, size) == -1) {
            perror("failed to set shared memory space size:");
            return 1;
        }

        //mmap to set up table
        item *table = mmap(NULL, SHM_SIZE*sizeof(item),
                            PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        //check for success
        if (table == (void *) -1) {
            perror("mmap failed :(");
        return 1;
        }

    }
    FILE * fp = fopen(argv[1], "r");
    if (fp == NULL){
        fprintf(stderr, "failed to open file %s", argv[1]);
    }
    //run the operation given in argv[2]
    if (argv[2] == 1) { //insert/update
        if (update_shm(fp, fd) != 0) {
            fprintf(stderr, "failed to insert/update shared memory");
            return 1;
        }
        else {
           printf("successfully updated shared memory with data from %s",
                    argv[1]);
        }
    }
    else if (argv[2] == 2) { //delete
        if (delete_shm(fp, fd) != 0) {
            fprintf(stderr, "failed to delete from shared memory");
            return 1;
        }
        else {
            printf("successfully deleted items specified in %s from shared memory",
                    argv[1]);
        }
    }
    else if (argv[2] == 3) { //print
        if (print_shm(fp, fd) != 0) {
            fprintf(stderr, "failed to print from shared memory");
            return 1;
        }
    }
    else {
        fprintf(stderr, "invalid operation: %s", argv[2]);
        return 1;
    }

    return 0;
}