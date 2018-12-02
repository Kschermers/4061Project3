/***********************************/
/*  CSCI 4061 F18
 *  Project 3
 *  Group 1
 *  Names: Nicole Walker, Samuel Ball, Kadin Schermers
 *  X500s: walk0760, ballx188, scher528
 */


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
pthread_cond_t request_exists  = PTHREAD_COND_INITIALIZER;
pthred_cond_t space_for_request = PTHREAD_COND_INITIALIZER;
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

/* ******************************* Questions ************************************/

//What is the mem stuff in addIntoCache ??

// "reset" the struct when you replace item in cache

//Is a FIFO apprach with 'start of list -> cyclic' appraoch effective?

//yes this is a good approach

//path is always local?

//no, but we just psuedo-service non local requests

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

void IncrementCacheNextToStore() {
    cache_next_to_store++;

    if (cache_next_to_store == cache_entries) {
        cache_next_to_store = 0;
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
      pthread_mutex_lock(&lock);
    // Accept client connection
      int fd = accept_connection():
    // Get request from the client
      char filename[1024];
      get_request(fd, filename);
    // Add the request into the queue
      struct request_t request = {fd, filename};
      while(req_next_to_store == req_next_to_retrieve){
          pthread_cond_wait(&lock, &space_for_request);
      }
      requests[req_next_to_store] = request;
      pthread_cond_signal(&request_exists);
      pthread_mutex_unlock(&lock);
   }
   return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {
  int start, stop, elapsed;
  int cache_idx;
  cache_entry_t current_entry;
  request_t current_req;

  while (1) {
    pthread_mutex_lock(&lock);

    // wait until request queue is not empty
    while (req_next_to_store == (req_next_to_retrieve + 1)) {
      pthread_cond_wait(&request_exists, &lock);
    }

    // Start recording time
    start = getCurrentTimeInMills();

    // Get the request from the queue
    current_req = requests[req_next_to_retrieve];
    // update index tracker for queue
    if(req_next_to_retrieve == (qlen-1)) {
      req_next_to_retrieve = 0;
    } else {
      req_next_to_retrieve++;
    }

    pthread_mutex_unlock(&lock);

    // a request has been handled so signal to a
    // dispatcher to handle a new one
    pthread_cond_signal(&space_for_request);

    // reacquire lock to begin working on cache
    pthread_mutex_lock(&lock);

    // TODO Get the data from the disk or the cache
    cache_idx = getCacheIndex(current_req.request);
    if (cache_idx != -1) {
      // req is in cache
      current_entry = cache[cache_idx];
    } else {
      // req is not in cache

    }

    // Stop recording the time
    stop = getCurrentTimeInMills();
    elapsed = stop - start;


    // Log the request into the file and terminal

    pthread_mutex_unlock(&lock);

    // return the result
    if () {
      return_result();
    } else {
      return_error();
    }


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
  num_dispatcher=atoi(argv[3]);
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
  tids[num_workers + num_dispatch];
    
    for (int i = 0; i < num_workers; i++) {
        tids[i] = i;
        pthread_create(&workers[i], NULL, worker, &tids[i]);
    }
    
    for(i = i; i < num_dispatch + i; i++) {
        tids[i] = i;
        pthread_create(&dispatchers[i], NULL, dispatch, &tids[i]);
    }

  // Clean up
  deleteCache();
  return 0;
}
