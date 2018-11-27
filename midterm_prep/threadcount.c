#include <stdio.h>
#include <pthread.h>


#define NUM_LOOPS 5000000
long long sum = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* counting_thread(void *arg)
{
	int offset = *(int *)arg;
	for (int i = 0; i < NUM_LOOPS; i++)
	{		
		pthread_mutex_lock(&mutex);

		sum += offset;

		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(NULL);
}

int main(void)
{

	pthread_t pt1;
	int offset1 = 1;
	pthread_create(&pt1, NULL, counting_thread, &offset1);

	pthread_t pt2;
	int offset2 = -1;
	pthread_create(&pt2, NULL, counting_thread, &offset2);

	pthread_join(pt1, NULL);
	pthread_join(pt2, NULL);

	printf("Sum: %lld\n",sum);
	
	return 0;
}
