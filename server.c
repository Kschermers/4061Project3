/***********************************/
/*  CSCI 4061 F18
 *  Project 3
 *  Group 1
 *  Names: Nicole Walker, Samuel Ball, Kadin Schermers
 *  X500s: walk0760, ballx188, scher528
 *  Date: 12/12/2018
 */

//hi
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include "util.h"
#include <stdbool.h>
#include <unistd.h>

#define MAX_THREADS 100
#define MAX_queue_len 100
#define MAX_CE 100
#define INVALID -1
#define BUFF_SIZE 1024
#define MAX_PATH 2048
#define MAX_ERROR 256
#define MAX_LOG 128

/*
 THE CODE STRUCTURE GIVEN BELOW IS JUST A SUGESSTION. FEEL FREE TO MODIFY AS NEEDED
 */
int port;
char* path;
int num_dispatch;
int num_workers;
int dynamic_flag;
int qlen;
int cache_entries;

// request and cache indices
int req_next_to_store = 0;
int req_next_to_retrieve = 0;
int cache_next_to_store = 0;
int req_current_items = 0;

pthread_mutex_t queuelock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cachelock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t loglock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t request_exists  = PTHREAD_COND_INITIALIZER;
pthread_cond_t space_for_request = PTHREAD_COND_INITIALIZER;


FILE* log_file;
int log_fd;
// structs:
typedef struct request_queue {
    int fd;
    char request[BUFF_SIZE];
} request_t;

typedef struct cache_entry {
    int flag;
    int len;
    char *request;
    char *content;
} cache_entry_t;

cache_entry_t *cache;
request_t *requests;

/* ******************************* Questions ************************************/

//TODO: Turn  in

/* ******************************************************************************/

/* ************************ Dynamic Pool Code ***********************************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
void * dynamic_pool_size_update(void *arg) {
    while(1) {
        // Run at regular intervals
        // Increase / decrease dynamically based on your policy
    }
}
/**********************************************************************************/

/* ************************************ Cache Code ********************************/

// Function to check whether the given request is present in cache
int getCacheIndex(char *request){
    int i;
    for(i = 0; i < cache_entries; i++) {

        if (cache[i].request != NULL) {

            if (strcmp(cache[i].request, request) == 0) {
                return i;
            }
        }
    }
    return -1;
}



// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){
    // It should add the request at an index according to the cache replacement policy
    // Make sure to allocate/free memeory when adding or replacing cache entries
    cache_entry_t toFree = cache[cache_next_to_store];

    if (toFree.flag == 1) {
        free(toFree.request);
        free(toFree.content);
    }

    toFree.request = (char*) malloc(strlen(mybuf));
    strcpy(toFree.request, mybuf);

    toFree.content = (char*) malloc(memory_size);
    memcpy(toFree.content, memory, memory_size + 1);

    toFree.len = memory_size;

    toFree.flag = 1;

    cache[cache_next_to_store] = toFree;

    cache_next_to_store = (cache_next_to_store + 1) % cache_entries;
}

// clear the memory allocated to the cache
void deleteCache(){
    int i;
    for (i = 0; i < cache_entries; i++) {
        free(cache[i].request);
        free(cache[i].content);
    }
    free(cache);
}

// Function to initialize the cache
void initCache(){
    cache = (cache_entry_t *) malloc(sizeof(cache_entry_t) * cache_entries);

    int i;
    for(i = 0; i < cache_entries; i++) {
        cache[i].flag = 0;
    }
}
void initQueue(){
    requests = (request_t *) malloc(sizeof(request_t) * qlen);
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(char *path,char **content, int *size) {

    FILE* file = fopen(path, "r");
    if (file == NULL) {

        *content = NULL;
        *size = -1;
        return -1;
    } else {


    // Determine length of file
    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate space for content
    *content = (char *) malloc(len+1);
    *size = len;

    // Read from file into content buffer
    fread(*content, len, 1, file);
    return 0;
    }
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
    int len = strlen(mybuf);

    // Should return the content type based on the file type in the request
    // (See Section 5 in Project description for more details)
    if(mybuf[len-5]=='.' && mybuf[len-4]=='h' && mybuf[len-3]=='t'&& mybuf[len-2]=='m'&& mybuf[len-1]=='l'){
        return "text/html";
    }
    else if(mybuf[len-4]=='.' && mybuf[len-3]=='j' && mybuf[len-2]=='p'&& mybuf[len-1]=='g'){
        return "image/jpg";
    }
    else if(mybuf[len-4]=='.' && mybuf[len-3]=='g' && mybuf[len-2]=='i'&& mybuf[len-1]=='f'){
        return "image/gif";
    }
    else{
        return "text/plain";
    }
}

// This function returns the current time in milliseconds
int getCurrentTimeInMills() {
    struct timeval curr_time;
    gettimeofday(&curr_time, NULL);
    return curr_time.tv_usec;
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {
    int fd;
    char filename[BUFF_SIZE];
    memset(filename,'\0',BUFF_SIZE);
    while (1) {
        // Accept client connection
        int tid = *(int *) arg;
        fd =  accept_connection();
        // Get request from the client
        if (get_request(fd, filename) == 0) {

            //Critical Section
            pthread_mutex_lock(&queuelock);
            while(req_current_items == qlen){
                pthread_cond_wait(&space_for_request, &queuelock);
            }

            requests[req_next_to_store].fd = fd;
            memset(requests[req_next_to_store].request,'\0',BUFF_SIZE);
            strncpy(requests[req_next_to_store].request, filename, BUFF_SIZE);
            req_current_items++;
            req_next_to_store = (req_next_to_store + 1) % qlen;
            pthread_mutex_unlock(&queuelock);
            pthread_cond_broadcast(&request_exists);
        }
    }
    return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {

    int start, stop, elapsed;
    int cache_idx;
    int thread_id = *(int *) arg;
    int req_num = 0;

    char cache_hit_miss[5];

    while (1) {

        pthread_mutex_lock(&queuelock);
        char *content;
        int contentBytes;
        char full_path[MAX_PATH];
        char bytes_error[MAX_ERROR];
        bool file_exists = false;
        char log_str[MAX_LOG];
        request_t current_req;
        // wait until request queue is not empty
        while (req_next_to_store == req_next_to_retrieve) {
            pthread_cond_wait(&request_exists, &queuelock);
        }

        // Start recording time
        start = getCurrentTimeInMills();

        // Get the request from the queue
        current_req.fd = requests[req_next_to_retrieve].fd;
        memset(current_req.request,'\0',BUFF_SIZE);
        strncpy(current_req.request, requests[req_next_to_retrieve].request,BUFF_SIZE);

        // update index tracker for queue
        req_current_items--;
        req_next_to_retrieve = (req_next_to_retrieve + 1) % qlen;
        pthread_mutex_unlock(&queuelock);

        pthread_cond_broadcast(&space_for_request);

        pthread_mutex_lock(&cachelock);

        // Get the data from the disk or the cache
        cache_idx = getCacheIndex(current_req.request);

        if (cache_idx!=-1) {

            // Req is in cache
            snprintf(cache_hit_miss, 4, "HIT");
            content = cache[cache_idx].content;
            contentBytes = cache[cache_idx].len;

            file_exists = true;
            memset(bytes_error, '\0', MAX_ERROR);
            sprintf(bytes_error,"%d",contentBytes);

            pthread_mutex_unlock(&cachelock);
        }


        else {
            pthread_mutex_unlock(&cachelock);
            // Req is not in cache

            snprintf(cache_hit_miss, 5, "MISS");

            strcpy(full_path, path);
            strcat(full_path, ((char *) current_req.request));
            if(readFromDisk(full_path,&content, &contentBytes) == 0){
                file_exists = true;
                pthread_mutex_lock(&cachelock);
                addIntoCache(current_req.request, content, contentBytes);
                
                pthread_mutex_unlock(&cachelock);
                
                memset(bytes_error, '\0', MAX_ERROR);
                sprintf(bytes_error,"%d",contentBytes);
            }
        }

        // Stop recording the time
        stop = getCurrentTimeInMills();
        elapsed = stop - start;

        // Return the result or set the error
        char * cType = getContentType(full_path);
        int retError;
        if ((retError = return_result(current_req.fd, cType, content, contentBytes)) != 0) {
            return_error(current_req.fd, content);
            if (!file_exists) {
              memset(bytes_error, '\0', MAX_ERROR);
              sprintf(bytes_error, "File does not exist");
            }
        }
        req_num++;

        pthread_mutex_lock(&loglock);

        snprintf(log_str, MAX_LOG, "[%d][%d][%d][%s][%s][%dms][%s]\n",
                 thread_id, req_num, current_req.fd, (char*) current_req.request,
                 bytes_error, elapsed, cache_hit_miss);
        int log_len = strlen(log_str);

        write(log_fd, log_str, log_len);

        // Log to terminal
        write(1, log_str, log_len);
        pthread_mutex_unlock(&loglock);
    }
    return NULL;
}

/**********************************************************************************/

int main(int argc, char **argv) {

    // Error check on number of arguments
    // Decided to check if caching is enabled [argc == 8 -> Caching enabled]
    if(argc != 8){
        printf("usage: %s port path num_dispatcher num_workers dynamic_flag queue_length cache_size\n", argv[0]);
        return -1;
    }

    // get the input arguments
    port = atoi(argv[1]);
    path = argv[2];
    num_dispatch=atoi(argv[3]);
    num_workers=atoi(argv[4]);
    dynamic_flag = atoi(argv[5]);
    qlen = atoi(argv[6]);
    cache_entries=atoi(argv[7]);

    // Perform error checks on the input arguments
    if(port < 1025 || port > 65535){
        printf("port #%d is invalid.\n", port);
        return -1;
    }
    if(num_workers > MAX_THREADS || num_workers <= 0){
        printf("num_workers #%d is invalid.\n", num_workers);
        return -1;
    }
    if(num_dispatch > MAX_THREADS || num_workers <= 0){
        printf("num_dispatchers #%d is invalid.\n", num_dispatch);
        return -1;
    }
    if(qlen > MAX_queue_len || qlen <= 0){
        printf("queue length #%d is invalid.\n", qlen);
        return -1;
    }
    if(cache_entries > MAX_CE || cache_entries <= 0){
        printf("cache entries #%d is invalid.\n", cache_entries);
        return -1;
    }
    // Change the current working directory to server root directory
    chdir(path);

    // Start the server and initialize cache
    init(port);
    initQueue();
    initCache();

    log_file = fopen("webserver_log.txt", "ab+");
    log_fd = fileno(log_file);
    // Create dispatcher and worker threads
    pthread_t dispatchers[num_dispatch];
    pthread_t workers[num_workers];
    int tids[num_workers + num_dispatch];

    int i;
    for (i = 0; i < num_dispatch; i++) {
        tids[i] = i;
        pthread_create(&dispatchers[i], NULL, dispatch, &tids[i]);
    }

    int j = i;
    for(; i < num_workers + j; i++) {
        tids[i] = i;
        pthread_create(&workers[i-j], NULL, worker, &tids[i]);
    }

    for (i = 0; i < num_dispatch; i++) {
        pthread_join(dispatchers[i], NULL);
    }
    j = i;
    for(; i < num_workers + j; i++) {
        pthread_join(workers[i-j], NULL);
    }

    // Clean up
    pthread_mutex_destroy(&queuelock);
    pthread_mutex_destroy(&cachelock);
    pthread_mutex_destroy(&loglock);
    pthread_cond_destroy(&request_exists);
    pthread_cond_destroy(&space_for_request);
    deleteCache();

    return 0;
}
