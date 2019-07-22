//main.c

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>
#include <sys/types.h>
#include <errno.h>

#define BUFF_SIZE 1024
#define SHM_SIZE 1024
#define SHM_KEY 12345

extern int errno;

typedef struct pair {
    int key;
    char val[BUFF_SIZE];
} item;

typedef struct shmspace {
    int num_items;
    item items[SHM_SIZE]; //1024 entries should suffice, right?
} space;


int main(int argc, char const *argv[]) {
    int shmid = shmget(SHM_KEY, sizeof(shmspace), 0664|IPC_CREAT|IPC_EXCL);
    if (shmid != -1) { //successfully created shared memory space
        /*
        not sure if required: will un-comment if so
        pid_t foo = fork();
        if (i == 0) {
            execve("./shmspace_host", shmid);
            //_exit()
        }
        */

        //attached to the segment to get a pointer to it
        space *foo = shmat(shmid, NULL, 0);
        if (shmspace == (void *) -1) {
            perror("Failed shared memory attach");
            return 1;
        }

        //run input operation given in argv[2]
        if (argv[2] == 1) { //insert/update

        }
        else if (argv[2] == 2) { //delete

        }
        else if (argv[2] == 3) { //print

        }
        else {
            perror("invalid operation")
        }

    }
    else { //shmid == -1
        if (strerror(errno) == "EEXIST") { //shared memory space already existed

        }
        else {
            perror("failed to initialize shared memory");
            return 1;
        }
    }

    /*
    if shmat() == fail {
        shmid = shmget()
    }
    fd = fopen(args[1]);
    int cmd = args[2];

    if cmd ==1
        perform insertion
    else if cmd == 2
        perform deletion
    else if cmd == 3
        perform print
    else
        error
    */
  return 0;
}
