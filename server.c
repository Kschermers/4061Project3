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
#include <time.h>
#include "util.h"
#include <stdbool.h>

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
int num_dispatcher;
int num_workers;
int dynamic_flag;
int qlen;
int cache_entries;

// request and cache indices
int req_next_to_store = 0;
int req_next_to_retrieve = -1;
int cache_next_to_store = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t request_cv  = PTHREAD_COND_INITIALIZER;
pthread_cond_t cache_cv  = PTHREAD_COND_INITIALIZER;

// structs:
typedef struct request_queue {
   int fd;
   void *request;
} request_t;

typedef struct cache_entry {
    int len;
    char *request;
    char *content;
} cache_entry_t;

cache_entry_t *cache;
request_t *requests;

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
    for(int i = 0; i < cache_entries; i++) {
        if (strcmp(cache[i].request,request) == 0) {
            return i;
        }
    }
    return -1;
}

// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memeory when adding or replacing cache entries
    
}

// clear the memory allocated to the cache
void deleteCache(){
    free(cache);
}

// Function to initialize the cache
void initCache(){
    cache = (cache_entry_t *) malloc(sizeof(cache_entry_t) * cache_entries);
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(/*necessary arguments*/) {
  // Open and read the contents of file given the request
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
    else if(mybuf[len-5]=='.' && mybuf[len-4]=='j' && mybuf[len-3]=='p'&& mybuf[len-2]=='e'&& mybuf[len-1]=='g'){
        return "image/jpeg";
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

  while (1) {

    // Accept client connection

    // Get request from the client

    // Add the request into the queue

   }
   return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {
  int start, stop, elapsed;
  request_t current;

  while (1) {
    // Start recording time
    start = getCurrentTimeInMills();

    // Get the request from the queue


    // Get the data from the disk or the cache
    (getCacheIndex(&current) != -1)

    // Stop recording the time
    stop = getCurrentTimeInMills();
    elapsed = stop - start;


    // Log the request into the file and terminal
    printf();

    // return the result
  }
  return NULL;
}

/**********************************************************************************/

int main(int argc, char **argv) {

  // Error check on number of arguments
  // Decided to check if caching is enabled [argc == 8 -> Caching enabled]
  if(argc != 7 && argc != 8){
    printf("usage: %s port path num_dispatcher num_workers dynamic_flag queue_length cache_size\n", argv[0]);
    return -1;
  }

  // Get the input args
    if(argc==7){
        port = atoi(argv[1]);
        path = argv[2];
        num_dispatcher=atoi(argv[3]);
        num_workers=atoi(argv[4]);
        dynamic_flag = 0;
        qlen = atoi(argv[5]);
        cache_entries=atoi(argv[6]);
    }
    else if(argc==8){
        port = atoi(argv[1]);
        path = argv[2];
        num_dispatcher=atoi(argv[3]);
        num_workers=atoi(argv[4]);
        dynamic_flag = atoi(argv[5]);
        qlen = atoi(argv[6]);
        cache_entries=atoi(argv[7]);
    }
  // Perform error checks on the input arguments
    if(port < 1025 || port > 65535){
        printf("port #%d is invalid.\n", port);
        return -1;
    }
    if(num_workers > MAX_THREADS || num_workers <= 0){
        printf("num_workers #%d is invalid.\n", num_workers);
        return -1;
    }
    if(num_dispatcher > MAX_THREADS || num_workers <= 0){
        printf("num_dispatchers #%d is invalid.\n", num_dispatcher);
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
  initCache();

  // Create dispatcher and worker threads
  pthread_t dispatchers[num_dispatch];
  pthread_t workers[num_workers];

  // Clean up
<<<<<<< HEAD

=======
    deleteCache();
>>>>>>> 8b4aef9e04ecef7803f2e5d0b4afc97968709946
  return 0;
}
