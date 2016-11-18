/***********************************************
* Concurrency 4.1 - Mutual Exclusive Threads   *
* Christian Armatas & Mihai Dan     		   *
* 11/17/2016             					   *
***********************************************/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define NUM_THREADS 10

sem_t mutex;
sem_t block_sem;

int active_thread;
int waiting_thread;
int wait = 0;

pthread_t threads[10];

void *thread_func(void *arg);

int main(){
	int i;
	
	/* Initialize the semaphores */
	sem_init(&mutex, 0, 1);
	sem_init(&block_sem, 0, 0);
	
	/* Create the threads */
	for(i = 0; i < NUM_THREADS; i++){
		pthread_create(&threads[i], NULL, thread_func, NULL);
	}
	
	/* Join the threads */
	for(i = 0; i < NUM_THREADS; i++){
		pthread_join(threads[i], NULL);
	}
	
	return 0;
}

void *thread_func(void *arg){
	while(1){
		sem_wait(&mutex);
		
		if(wait){
			waiting_thread++;
			sem_post(&mutex);
			
			printf("All users departing.\n");
			
			sem_wait(&block_sem);
			waiting_thread--;
		}
		
		/* Once the mutex is available, a thread becomes active... */
		active_thread++;
		printf("Threads currently active: %d\n", active_thread);
		
		/* Check to see if there are three active users */
		if(active_thread == 3){
			sleep(3);
			wait = 1;
		}else{
			wait = 0;
		}
		
		/* If threads are waiting, but not yet 3... */
		if(waiting_thread > 0 && wait == 0){
			/* ... unblock a thread. */
			sem_post(&block_sem);
			printf("Thread unblocked.\n");
		}else{
			/* Open mutual exclusion for the thread */
			sem_post(&mutex);
			printf("Mutex posted for a thread.\n");
		}
		
		/* Wait on mutex */
		sem_wait(&mutex);
		
		/* One less user is active */
		active_thread--;
		
		/* Check for no threads */
		if(active_thread == 0){
			printf("There are no active threads.\n");
			wait = 0;
		}
		
		/* If threads are waiting, but not yet 3... */
		if(waiting_thread != 0 && wait == 0){
			/* ... unblock a thread. */
			sem_post(&block_sem);
			printf("Thread unblocked.\n");
		}else{
			/* Open mutual exclusion for the thread */
			sem_post(&mutex);
			printf("Mutex posted for a thread.\n");
		}
	}
}

/* References:
 * https://lab.cs.ru.nl/algemeen/images/8/8f/Opgavenserie9.pdf
 */