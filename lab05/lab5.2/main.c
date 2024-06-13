
#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>

#include <pthread.h>



#define MAX_QUEUE_SIZE 25
#define MIN_QUEUE_SIZE 5
int CURRENT_MAX_QUEUE_SIZE = 10;


#define MAX_CHILD_COUNT 5


pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;

pthread_t consumers[MAX_CHILD_COUNT];
char* consumers_name[MAX_CHILD_COUNT];
int consumers_count;
pthread_t producers[MAX_CHILD_COUNT];
char* producers_name[MAX_CHILD_COUNT];
int producers_count;

typedef struct{
    uint8_t type;
    uint16_t hash;
    uint8_t size;
    char* data;
} message;

typedef struct{
    message* head;
    int h;
    message* tail;
    int t;
    message* buff;
    int count_added;
    int count_extracted;
}queue;

queue *message_queue;

uint8_t getSize(){
    int size = 0;
    while(size == 0)size = rand() % 257;
    if(size == 256)size=0;
    return size;
}

uint8_t getType(uint8_t size){
    if(size>128)return 1;
    else return 0;
}

char* getData(uint8_t size){
    if(size==0)size=(uint8_t)256;
    char* data = (char*) malloc(size);
    if(data==NULL)perror("queue message data");
    srand(time(NULL));
    for(int i = 0; i < size; i++)data[i] = (rand() % 26) + 'a';
    return data;
}

uint16_t FNV1_HASH(const void *data, size_t size) {
    const uint8_t *bytes = (const uint8_t *) data;
    uint16_t hash = 0x811C;
    for (size_t i = 0; i < size; ++i) {
        hash = (hash * 0x0101) ^ bytes[i];
    }
    return hash;
}

message* createMessage(){

    message* msg;
    msg = (message *) malloc(sizeof(*msg));
    msg->size = getSize();
    msg->data = getData(((msg->size + 3) / 4) * 4);
    msg->type = getType(msg->size);
    msg->hash = 0;
    msg->hash = FNV1_HASH(msg->data, strlen(msg->data));

    return msg;
}

void deleteConsumers(){
    if(consumers_count == 0){
        printf("No consumers.");
        return;
    }
    consumers_count--;
    pthread_cancel(consumers[consumers_count]);
    printf("Was delete consumer with name:%s\n",consumers_name[consumers_count]);
    free(consumers_name[consumers_count]);
}

void deleteProducers(){
    if(producers_count == 0){
        printf("No consumers.");
        return;
    }
    producers_count--;
    pthread_cond_signal(&full);
    pthread_cancel(producers[producers_count]);
    printf("Was delete producer with name:%s\n",producers_name[producers_count]);
    free(producers_name[producers_count]);
}

void start() {

    message_queue = malloc(sizeof(queue));
    if(message_queue == NULL) {
        perror("Failed to allocate memory for message queue");
        exit(EXIT_FAILURE);
    }

    memset(message_queue, 0, sizeof(queue));

    message_queue->buff = (message *)malloc(15 * sizeof(message));

}

void fromProgExit(){
    while (consumers_count) deleteConsumers();
    while (producers_count) deleteProducers();


    free(message_queue);

    exit(0);
}

void viewStatus(){
    printf("Queue max size:%d\n",CURRENT_MAX_QUEUE_SIZE);
    printf("Current size:%d\n",message_queue->count_added-message_queue->count_extracted);
    printf("Added:%d \nExtracted:%d\n",message_queue->count_added,message_queue->count_extracted);
    printf("Consumers:%d\n",consumers_count);
    printf("Producers:%d\n",producers_count);
}

int addMessage(message* msg){

    pthread_mutex_lock(&mutex1);
    while((message_queue->count_added-message_queue->count_extracted)>=CURRENT_MAX_QUEUE_SIZE){
        pthread_cond_wait(&full, &mutex1);
    }

    if(message_queue->count_added-message_queue->count_extracted==0){
        message_queue->buff[0] = *msg;
        message_queue->head = message_queue->tail = &message_queue->buff[0];
    }else message_queue->t++;

    message_queue->count_added++;
    message_queue->t = message_queue->t%CURRENT_MAX_QUEUE_SIZE;
    message_queue->buff[message_queue->t]=*msg;
    message_queue->tail = &message_queue->buff[message_queue->t];

    pthread_cond_signal(&full);
    pthread_mutex_unlock(&mutex1);
    return message_queue->count_added;
}

int extractedMessage(message **msg){

    pthread_mutex_lock(&mutex2);

    while(message_queue->count_added == message_queue->count_extracted){
        pthread_cond_wait(&full,&mutex2);
    }
    *msg = message_queue->head;
    message_queue->count_extracted++;
    message_queue->h++;
    message_queue->h = message_queue->h % CURRENT_MAX_QUEUE_SIZE;
    message_queue->head = &message_queue->buff[message_queue->h];

    pthread_cond_signal(&full);
    pthread_mutex_unlock(&mutex2);

    return message_queue->count_extracted;
}

void *consumer_func(){
    message* msg;
    char name[20];
    sprintf(name, "consumer_%02d", consumers_count);
    while(1){
        int ret = extractedMessage(&msg);
        if(ret != -1){
            printf("%s consumer message: HASH=%04X, counter_extracted=%d\n", name, msg->hash, ret);
        }
        sleep(5);
    }
    return NULL;
}

void *producer_func(){
    message* msg;
    char name[20];
    sprintf(name, "producer%02d", producers_count);
    while(1){
        msg = createMessage();
        int ret = addMessage(msg);
        if(ret != -1){
            printf("%s producer message: HASH=%04X, counter_added=%d\n", name, msg->hash, ret);
        }
        sleep(5);
    }
    return NULL;
}


void addConsumer(){
    if(consumers_count==MAX_CHILD_COUNT){
        printf("Max consumer size\n");
        return;
    }
    pthread_create(&consumers[consumers_count], NULL, consumer_func, NULL);
    consumers_name[consumers_count] = (char *) malloc(16);
    sprintf(consumers_name[consumers_count], "consumer_%02d", consumers_count);
    consumers_count++;
}

void addProducer(){
    if(producers_count==MAX_CHILD_COUNT){
        printf("Max producer size\n");
        return;
    }
    pthread_create(&producers[producers_count], NULL, producer_func, NULL);
    producers_name[producers_count] = (char *) malloc(16);
    sprintf(producers_name[producers_count], "producer%02d", producers_count);
    producers_count++;
}

void menu() {
    printf("---------------------------------------\n");
    printf("'1' - to create producer.\n");
    printf("'2' - to remove producer.\n");
    printf("'3' - to create consumer.\n");
    printf("'4' - to remove consumer.\n");
    printf("'p' - viev processes.\n");
    printf("'s' - viev status.\n");
    printf("'-' - to decrement queue size.\n");
    printf("'+' - to increment queue size.\n");
    printf("'q' - to exit.\n");
    printf("---------------------------------------\n");
    
}

void viewProcesses(){
    if(consumers_count == 0 && producers_count == 0){
        return;
    }
    for(int i = 0; i < consumers_count; i++) printf("%s\n",consumers_name[i]);
    for(int i = 0; i < producers_count; i++) printf("%s\n",producers_name[i]);
}



void dec_queue_func(){

    if(CURRENT_MAX_QUEUE_SIZE == MIN_QUEUE_SIZE){
        printf("Min queue size\n");
        return;
    }

    message* new_buff = (message*)malloc((CURRENT_MAX_QUEUE_SIZE - 1) * sizeof(message));
    if (new_buff == NULL) {
        perror("Memory with decrement queue.\n");
        return;
    }
    int new_cnt_size;

    if((message_queue->count_added - message_queue->count_extracted) == 0){
        message_queue->h = message_queue->t = 0;
    }else{

        if((message_queue->count_added - message_queue->count_extracted) == CURRENT_MAX_QUEUE_SIZE){
            message_queue->count_added--;//
            new_cnt_size = CURRENT_MAX_QUEUE_SIZE - 1;
        }else{
            new_cnt_size = message_queue->count_added - message_queue->count_extracted;
        }

        for(int i = 0; i < new_cnt_size; i++){
            new_buff[i] = message_queue->buff[message_queue->h];
            message_queue->h++;
            message_queue->h = message_queue->h % CURRENT_MAX_QUEUE_SIZE;
        }

        message_queue->h = 0;
        message_queue->t = new_cnt_size - 1;

    }

    CURRENT_MAX_QUEUE_SIZE--;
    free(message_queue->buff);
    message_queue->buff = new_buff;

}


void inc_queue_func(){

    if(CURRENT_MAX_QUEUE_SIZE == MAX_QUEUE_SIZE){
        printf("Max queue size\n");
        return;
    }

    message* new_buff = (message*)malloc((CURRENT_MAX_QUEUE_SIZE + 1) * sizeof(message));

    if (new_buff == NULL) {
        perror("Memory with decrement queue.\n");
        return;
    }

    CURRENT_MAX_QUEUE_SIZE++;
    free(message_queue->buff);
    message_queue->buff = new_buff;
    pthread_cond_signal(&full);

}

int main(){
    start();

    printf("Write 'u' to see usage.\n");

    while (1) {
        char sumbol;
        rewind(stdin);
        sumbol = getchar();
        switch (sumbol) {
            case '1': {
                addProducer();
                break;
            }
            case '2': {
                deleteProducers();
                break;
            }
            case '3': {
                addConsumer();
                break;
            }
            case '4': {
                deleteConsumers();
                break;
            }
            case 'p': {
                viewProcesses();
                break;
            }
            case 'q': {
                fromProgExit();
                break;
            }
            case 's': {
                viewStatus();
                break;
            }
            case 'u': {
                menu();
                break;
            }
            case '-': {
                dec_queue_func();
                break;
            }
            case '+': {
                inc_queue_func();
                break;
            }
            default:
                break;
        }
    }
fromProgExit();
    return 0;
}