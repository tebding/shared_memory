//shmspace_host.c
//idle process to maintain the shared memory space

#define SHM_SIZE 1024
#define BUFF_SIZE 1024

struct item {
    int key;
    char val[BUFF_SIZE];
};

struct shmspace {
    int num_items;
    item items[SHM_SIZE]; //1024 entries should suffice...
};

int int main(int argc, char const *argv[]) {
    int shmid = argv[1];
    struct shmspace *shmp = shmat(shmid, NULL, 0);
    while(1);
    return 0;
}
