/***********************************/
/*  CSCI 4061 F18
 *  Project 3
 *  Group 1
 *  Names: Nicole Walker, Samuel Ball, Kadin Schermers
 *  X500s: walk0760, ballx188, scher528
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
int req_next_to_retrieve = -1;
int cache_next_to_store = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
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

//TODO: understand content versus request


//all we care about is the readout from the file no matter what it is,
//request -> read cache/disk -> content -> return to web
//fstat = number of bytes of a file -> this as return of readFromDisk?

//TODO: complete readFromDisk()

//TODO: other stuff

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
    if (strcmp(cache[i].request,request) == 0) {
      return i;
    }
  }
  return -1;
}

void IncrementCacheCounter() {
    cache_next_to_store++;

    if (cache_next_to_store == cache_entries) {
        cache_next_to_store = 0;
    }
}

// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memeory when adding or replacing cache entries
     pthread_mutex_lock(&lock);

     cache_entry_t toFree = cache[cache_next_to_store];

     if (toFree.flag == 1) {
         free(toFree.request);
          free(toFree.content);
     }

     toFree.request = malloc(strlen(mybuf));
     toFree.request = mybuf;
     toFree.content = malloc(strlen(memory));
     toFree.content = memory;
     toFree.len = memory_size;

     toFree.flag = 1;

     cache[cache_next_to_store] = toFree;
     IncrementCacheCounter();

     pthread_mutex_unlock(&lock);
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
char* readFromDisk(char *path) {
     struct stat filestats;
     FILE *file = fopen(path,"r");
    int fd = fileno(file);
     if (fstat(fd,&filestats) < 0) {
         printf("File at %s was unable to be read\n",path);
         return NULL;
     } else {
         //man fstat to understand what this is doing
         int bytes = filestats.st_size;

         char *fileContent = (char *) malloc(bytes);
         read(fd,fileContent,bytes);
      return fileContent;
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
  int fd;
    char filename[1024];
    memset(filename,'\0',1024);
  while (1) {
    pthread_mutex_lock(&lock);
    // Accept client connection
    int tid = *(int *) arg;
    printf("DEBUG: TID #%d Attempting Connection\n", tid);
    fd =  accept_connection();
    // Get request from the client
    

    if (get_request(fd, filename) == 0) {
      printf("DEBUG: TID #%d get_request() succeeded\n", tid);
      while(req_next_to_store == req_next_to_retrieve){
        printf("DEBUG: TID #%d waiting for space in request queue\n", tid);
        pthread_cond_wait(&space_for_request, &lock);
      }
      printf("DEBUG: TID #%d putting request into queue\n", tid);
        requests[req_next_to_store].fd = fd;
        memset(requests[req_next_to_store].request,'\0',1024);
        strncpy(requests[req_next_to_store].request, filename, 1024);
      printf("DEBUG: TID #%d successfully put request into queue\n", tid);
      pthread_cond_signal(&request_exists);
    } else {
      printf("DEBUG: TID #%d get_request() failed\n", tid);
    }

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

  // temp variables for logging
  char log_str[128];
  int thread_id = *(int *) arg;
  int req_num = 0;
  char bytes_error[256];
  char cache_hit_miss[5];
  char* content = NULL;
  int contentBytes;

  while (1) {
    pthread_mutex_lock(&lock);
    req_num++;

    // wait until request queue is not empty
    while (req_next_to_store == (req_next_to_retrieve + 1)) {
      printf("DEBUG: TID #%d Waiting for a request\n", thread_id);
      pthread_cond_wait(&request_exists, &lock);
      printf("DEBUG: TID #%d recieved signal\n", thread_id);
    }
    printf("DEBUG: TID #%d handling request\n", thread_id);

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
    printf("DEBUG: TID #%d got request out of queue\n", thread_id);

    // a request has been handled so signal to a
    // dispatcher to handle a new one
    pthread_cond_signal(&space_for_request);


    // Get the data from the disk or the cache
    cache_idx = getCacheIndex(current_req.request);
    if (cache_idx != -1) {
      // Req is in cache
      snprintf(cache_hit_miss, 4, "HIT");
      current_entry = cache[cache_idx];
    } else {
      // Req is not in cache
      snprintf(cache_hit_miss, 5, "MISS");

      content = readFromDisk(current_req.request);
      contentBytes = strlen(content);

      addIntoCache(current_req.request,content,contentBytes);

      current_entry = cache[cache_next_to_store-1];
    }

    // Stop recording the time
    stop = getCurrentTimeInMills();
    elapsed = stop - start;



    // Return the result or set the error
    char * cType = getContentType(content);
    if (return_result(current_req.fd, cType, content, contentBytes) != 0) {
      return_error(current_req.fd, bytes_error);
    } else {
      sprintf(bytes_error, "%d", contentBytes);
    }



    // TODO Log the request into the file and terminal
    printf("DEBUG: before logging in worker\n");
    snprintf(log_str, 256, "[%d][%d][%d][%s][%s][%dms][%s]",
             thread_id, req_num, current_req.fd, (char*) current_req.request,
             bytes_error, elapsed, cache_hit_miss);
    int log_len = strlen(log_str);

    // TODO finish logging
    // Log to file
    FILE* log_file = fopen("webserver_log.txt", "w");
    int log_fd = fileno(log_file);
    write(log_fd, log_str, log_len);

    // Log to terminal
    write(1, log_str, log_len);

    pthread_mutex_unlock(&lock);
    req_num++;
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

  // Create dispatcher and worker threads
  pthread_t dispatchers[num_dispatch];
  pthread_t workers[num_workers];
  int tids[num_workers + num_dispatch];

  int i;
  for (i = 0; i < num_workers; i++) {
    tids[i] = i;
    pthread_create(&workers[i], NULL, worker, &tids[i]);
  }

  int j = i;
  for(; i < num_dispatch + j; i++) {
    tids[i] = i;
    pthread_create(&dispatchers[i-j], NULL, dispatch, &tids[i]);
  }

  for (i = 0; i < num_dispatch; i++) {
    pthread_join(dispatchers[i], NULL);
  }
  j = i;
  for(; i < num_workers + j; i++) {
    pthread_join(workers[i-j], NULL);
  }

  // Clean up
  pthread_mutex_destroy(&lock);
  pthread_cond_destroy(&request_exists);
  pthread_cond_destroy(&space_for_request);
  pthread_cond_destroy(&cache_cv);
  deleteCache();
  return 0;
}
