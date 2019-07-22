//main.c

#include <string.h>
#include <unistd.h> //probably not necessary?
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h> //for O_* constants
#include <sys/stat.h> //for mode constants
#include <errno.h>

#define BUFF_SIZE 1024
#define SHM_SIZE 1024
//#define SHM_KEY 12345

extern int errno;

typedef struct pair {
    int key;
    char val[BUFF_SIZE];
} item;

typedef struct shmspace {
    int num_items;
    item items[SHM_SIZE]; //1024 entries should suffice, right?
} space;


int update_shm(FILE *fp) {
    /*
        1) tokenize input -> 1st=key, 2nd=val
        2) use mmap (+ buffers?) to write to shared memory: helper fn
            a) hash the data
            b) copy in/flush whatever
            c) check for success
        3) ???
    */
}

int delete_shm(FILE *fp) {
    /*
        1) tokenize input, collect all keys
        2) check hash table for keys.
        3) for the keys, delete the pair.
        4) check for success?
    */
}

int print_shm(FILE *fp) {
    /*
        1) tokenize input, collect all keys
        2) check hash table for keys
        3) printf() associated values
    */
}


int main(int argc, char const *argv[]) {
    char *p;
    /*
        I opted against combining the shm_open() calls in order
        to eliminate the otherwise-inevitable calls to ftruncate()
        that are entirely unnecessary except for when O_CREAT
        is invoked.
    */
    int fd = shm_open("shm", O_RDWR);
    if (fd == -1) { //memory space does not yet exist
        fd = shm_open("shm", O_CREAT, 0666);
        if (fd == -1) { //failed to create memory space
            perror("failed to open/create");
            return 1;
        }
        //set the space to the desired size
        if (ftruncate(fd, sizeof(space)) == -1) {
            perror("failed to set shared memory space size");
            return 1;
        }
    }

    p = mmap(NULL, sizeof(space), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == (void *) -1) {
        perror("mmap failed :(");
        return 1;
    }
    FILE * fp = fopen(argv[1], "r");
    if (fp == NULL){
        fprintf(stderr, "failed to open file %s", argv[1]);
    }
    //run the operation given in argv[2]
    if (argv[2] == 1) { //insert/update
        if (update_shm(fp) != 1) {
            fprintf(stderr, "failed to insert/update shared memory");
        }
    }
    else if (argv[2] == 2) { //delete
        if (delete_shm(fp) != 1) {
            fprintf(stderr, "failed to delete from shared memory");
        }
    }
    else if (argv[2] == 3) { //print
        if (print_shm(fp) != 1) {
            fprintf(stderr, "failed to print from shared memory");
        }
    }
    else {
        fprintf(stderr, "invalid operation: %d", argv[2]);
    }

    return 0;
}
