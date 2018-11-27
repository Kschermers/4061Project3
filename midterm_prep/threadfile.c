#include <pthread.h>

// global declarations here if needed

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// assume these are callable by your program

void put_cache (file_t *); // put file into disk_cache

file_t *get_cache(request_t *); // lookup file in disk_cache, NULL if not there

file_t *get_disk(request_t *); // returns file from disk

void send_file (file_t *); // send file to client

request_t* get_next_request (); //returns next request

void handle_request (void *arg) 
{
	request_t *R;
	file_t *file; // the file that will be sent to the user
	R = (request_t *) arg; / now you can use the argument!
	// fill in code here
	
	pthread_mutex_lock(&mutex);

	if (get_cache(R) != NULL) {
		file = get_cache(R);
	}
	pthread_mutex_unlock(&mutex);
	if (file == NULL) {
		file = get_disk(R);
		pthread_mutex_lock(&mutex);
		put_cache(file);
		pthread_mutex_unlock(&mutex);
	}
	
	free (arg);
	send_file (file); // sends file to client
	pthread_exit (NULL);
}


void main () 
{
	request_t *R; // more declarations below
	pthread_t id[2];
	int count = 0;
	
	while (1) 
	{
		R = get_next_request (); // req is allocated on the heap
		// fill in code here
			
		pthread_create(&id[count], NULL, handle_request, &R);
		count = (count + 1) % 2;
		
	}
}
