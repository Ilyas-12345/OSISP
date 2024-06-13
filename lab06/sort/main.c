#include "sort_index.h"

pthread_barrier_t barrier;
pthread_mutex_t mutex;
index_record* cur;

int size;
int blocks;
int threads;

int main(int argc, char* argv[]) {
    if (argc > 1) {

        printf("Scanning params.\n");
        size = atoi(argv[1]);
        blocks = atoi(argv[2]);
        threads = atoi(argv[3]);
        
        if (size % 4096 != 0) {
            printf("Wrong size, it should be multple 4096.\n");
            return 0;
        }
        
        if (threads < 8 || threads > 8000) {
            printf("Wrong count of threads, form 8 to 8000.\n");
            return 0;
        }
        
        if (blocks % 2 != 0 || blocks < threads) {
            printf("Wrong count of block, it should be multiple 2 and more than threads count.\n");
            return 0;
        }

        if (pthread_barrier_init(&barrier, NULL, threads)) {
            printf("Error while creating barrier.\n");
            return 0;
        }

        if (pthread_mutex_init(&mutex, NULL) != 0) {
            printf("Error while creating mutex.\n");
            return 0;
        }

        creatingData* cd = (creatingData*)malloc(sizeof(creatingData));

        cd->blockSize = size/blocks;
        cd->threads = threads;
        cd->fileName = argv[4];

        pthread_t pthreadId;
        if (pthread_create(&pthreadId, NULL, openMem, cd) != 0) {
            printf("Error while creating 0 thread.\n");
            exit(0);
        }

        if(pthread_join(pthreadId, NULL) != 0) {
            printf("Error executing process.\n");
            exit(0);
        }

        pthread_barrier_destroy(&barrier);
        pthread_mutex_destroy(&mutex);
        printf("Work done.\n");
    
    } else {
        printf("Not enough parameters.\n");
    }
    
    return 0;
}