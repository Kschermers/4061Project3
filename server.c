/***********************************/
/*  CSCI 4061 F18
 *  Project 3
 *  Group 1
 *  Names: Nicole Walker, Samuel Ball, Kadin Schermers
 *  X500s: walk0760, ballx188, scher528
 *  Date: 01/01/1970
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
pthread_cond_t request_exists  = PTHREAD_COND_INITIALIZER;
pthread_cond_t space_for_request = PTHREAD_COND_INITIALIZER;
pthread_cond_t cache_cv  = PTHREAD_COND_INITIALIZER;

// structs:
typedef struct request_queue {
    int fd;
    char request[1024];
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

//TODO: When returnFromDisk retuerns NULL, fix hang

//TODO: read images/gifs properly


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
        printf("DEBUG: getCacheIndex(): comparing to index %d in cache\n", i);
        if (cache[i].request != NULL) {
            printf("DEBUG: content of cache request: %s at cache index %d\n",cache[i].request,i);
            printf("DEBUG: content of incoming request: %s\n",request);
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
    printf("DEBUG: cache index %d assigned new request: %s\n",cache_next_to_store,toFree.request);
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
    //  struct stat filestats;
    printf("DEBUG: readFromDisk(): path %s being opened...\n", path);
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        printf("DEBUG: readFromDisk(): fopen() returned NULL\n");
        
        *content = NULL;
       
        *size = -1;
        return -1;
    } else {
        printf("DEBUG: readFromDisk(): fopen() returned file\n");
    }
    // Determine length of file
    fseek(file, 0, SEEK_END);
    long len = ftell(file);
    fseek(file, 0, SEEK_SET);
    printf("DEBUG: readFromDisk(): Length of file is %d\n", len);

    // Allocate space for content
    *content = (char *) malloc(len+1);
    
    printf("DEBUG: content malloced in RFD\n");
    *size = len;
    
    // Read from file into content buffer
    fread(*content, len, 1, file);
    printf("DEBUG: content read from file\n");
    *content[len] = '\0';

    //printf("DEBUG: readFromDisk(): Length written to file is %d\n", le);

    return 0;
    //}
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
    int len = strlen(mybuf);
    if (mybuf == NULL) {
        printf("DEBUG: getContentType(): input is NULL");
    } else {
        printf("DEBUG: Getting content type of %s\n", mybuf);
    }

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
    char filename[1024];
    memset(filename,'\0',1024);
    while (1) {
        // Accept client connection
        int tid = *(int *) arg;
        // printf("DEBUG: DISPATCH TID #%d Attempting Connection\n", tid);
        fd =  accept_connection();
        // Get request from the client
        if (get_request(fd, filename) == 0) {
            //printf("DEBUG: DISPATCH TID #%d get_request() succeeded\n", tid);

            //Critical Section
            pthread_mutex_lock(&queuelock);
            while(req_current_items == qlen){
                pthread_cond_wait(&space_for_request, &queuelock);
            }

            requests[req_next_to_store].fd = fd;
            memset(requests[req_next_to_store].request,'\0',1024);
            strncpy(requests[req_next_to_store].request, filename, 1024);
            req_current_items++;
            req_next_to_store = (req_next_to_store + 1) % qlen;
            // printf("DEBUG: DISPATCH TID #%d successfully put request into queue\n", tid);
            pthread_mutex_unlock(&queuelock);
            pthread_cond_broadcast(&request_exists);

        } else {
            // printf("DEBUG: DISPATCH TID #%d get_request() failed\n", tid);
        }
    }
    return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {
    int start, stop, elapsed;
    int cache_idx;


    // temp variables for logging
    char log_str[128];
    int thread_id = *(int *) arg;
    int req_num = 0;
    char bytes_error[256];
    char cache_hit_miss[5];
    
    while (1) {
        pthread_mutex_lock(&queuelock);
        char *content;
        int contentBytes;
        char full_path[2048];
        request_t current_req;
        // wait until request queue is not empty
        while (req_next_to_store == req_next_to_retrieve) {
            // printf("DEBUG: WORKER TID #%d Waiting for a request\n", thread_id);
            pthread_cond_wait(&request_exists, &queuelock);
        }
        printf("DEBUG: WORKER TID #%d handling request\n", thread_id);

        // Start recording time
        start = getCurrentTimeInMills();

        // Get the request from the queue
        current_req.fd = requests[req_next_to_retrieve].fd;
        memset(current_req.request,'\0',1024);
        strncpy(current_req.request, requests[req_next_to_retrieve].request, 1024);

        printf("DEBUG: WORKER TID #%d got request out of queue\n", thread_id);
        // update index tracker for queue
        req_current_items--;
        req_next_to_retrieve = (req_next_to_retrieve + 1) % qlen;
        pthread_mutex_unlock(&queuelock);

        pthread_cond_broadcast(&space_for_request);

        pthread_mutex_lock(&cachelock);
        // Get the data from the disk or the cache
        printf("DEBUG: WORKER TID #%d trying to get cache index\n", thread_id);
        cache_idx = getCacheIndex(current_req.request);
        printf("DEBUG: WORKER TID #%d got cache index\n", thread_id);
        if (cache_idx!=-1) {
            printf("DEBUG: WORKER TID #%d request is in cache\n", thread_id);
            // Req is in cache
            snprintf(cache_hit_miss, 4, "HIT");
            content = cache[cache_idx].content;
            contentBytes = cache[cache_idx].len;
        }
        
        else {
            // Req is not in cache
            printf("DEBUG: WORKER TID #%d request is NOT in cache\n", thread_id);
            snprintf(cache_hit_miss, 5, "MISS");
            printf("DEBUG: WORKER TID #%d trying to get request from disk\n", thread_id);
            
            strcpy(full_path, path);
            strcat(full_path, ((char *) current_req.request));
            if(readFromDisk(full_path,&content, &contentBytes) == 0){
                printf("DEBUG: WORKER TID #%d length of content found\n", thread_id);
                addIntoCache(current_req.request, content, contentBytes);
                printf("DEBUG: WORKER TID #%d added into cache\n", thread_id);
            }
        }

        // Stop recording the time
        stop = getCurrentTimeInMills();
        elapsed = stop - start;

        // Return the result or set the error
        char * cType = getContentType(full_path);
        if (return_result(current_req.fd, cType, content, contentBytes) != 0) {
            return_error(current_req.fd, bytes_error);
        } else {
            req_num++;
            sprintf(bytes_error, "%d", contentBytes);
        }
        printf("DEBUG: before logging in worker\n");
        snprintf(log_str, 256, "[%d][%d][%d][%s][%s][%dms][%s]\n",
                 thread_id, req_num, current_req.fd, (char*) current_req.request,
                 bytes_error, elapsed, cache_hit_miss);
        int log_len = strlen(log_str);
        FILE* log_file = fopen("webserver_log.txt", "w");
        int log_fd = fileno(log_file);
        write(log_fd, log_str, log_len);

        // Log to terminal
        write(1, log_str, log_len);
        pthread_mutex_unlock(&cachelock);
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
    // chdir(path);

    // Start the server and initialize cache
    init(port);
    initQueue();
    initCache();

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
    pthread_cond_destroy(&request_exists);
    pthread_cond_destroy(&space_for_request);
    pthread_cond_destroy(&cache_cv);
    deleteCache();
    return 0;
}
