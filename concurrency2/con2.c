/**************************************
* Concurrency 2 - Dining Philosophers *
* 	Christian Armatas & Mihai Dan     *
* 			   11/05/2016             *
**************************************/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "mt19937ar.h"

#define philo_max 5

// definition of philo_struct type
typedef struct{
		int name;
		sem_t *forks;
		sem_t *lock;
		int eat_count;
} philo_struct;

// function prototypes
void think(int name);
void get_forks(philo_struct *temp);
void put_forks(philo_struct *temp);
void eat(int name);
void *philo_loop();
void print_forks(sem_t frks[philo_max]);
void interrupt_handler();

// philosophers pthread array
pthread_t philosophers[philo_max];

/* think() function */
// Philosopher is assigned random think time in between 1 and 20 seconds.
// Thinks for random amount of time.
void think(int name){
	srand(time(NULL));
	
	int think_time = (genrand_int32() % 20) + 1;
	printf("Philosopher %d is thinking for %d seconds.\n", name+1, think_time);
	sleep(think_time);
}

/* eat() function */
// Philosopher is assigned random eat time in between 1 and 20 seconds.
// Eats for random amount of time.
void eat(int name){
	srand(time(NULL));
	
	int eat_time = (genrand_int32() % 7) + 2;
	printf("Philosopher %d is eating for %d seconds.\n", name+1, eat_time);
	sleep(eat_time);
}
/* get_forks() function */
// Waiting on lock semaphore.
// Checks forks on the left and right.
void get_forks(philo_struct *temp){
	sem_wait(temp->lock);
	sem_wait(&temp->forks[temp->name]);
	sem_wait(&temp->forks[(temp->name + 1) % philo_max]);
}

/* put_forks function() */
// Sets forks down.
// Resets lock semaphore value.
void put_forks(philo_struct *temp){
	sem_post(&temp->forks[temp->name]);
	sem_post(&temp->forks[(temp->name + 1) % philo_max]);
	sem_post(temp->lock);
}

/* *philo_loop() function */
// Performs philosopher actions.
void *philo_loop(void *arg){
	philo_struct temp = *(philo_struct *)arg;
	
	while(1){
		think(temp.name);		// call to think()
		get_forks(&temp);		// check for forks
		eat(temp.name);			// call to eat() when forks are avaiable
		++temp.eat_count;		// increment eat_count
		
		printf("Philosopher %d has eaten %d times.\n", temp.name+1, temp.eat_count);
		
		put_forks(&temp);		// put forks down
		print_forks(temp.forks);
	}
}

/* print_forks() function */
// Print the forks and their availability.
void print_forks(sem_t frks[philo_max]){
	int s_val, i;
	
	for(i = 0; i < philo_max; i++){
		sem_getvalue(&frks[i], &s_val);
		if(s_val == 1){
			printf("Fork %d is in use.\n", i);
		}else{
			printf("Fork %d is not in use.\n", i);
		}
	}
}

/* interrupt_handler() function */
void interrupt_handler(){
	int i;
	
	for(i = 0; i < philo_max; i++){
		pthread_detach(philosophers[i]);
	}
	exit(EXIT_SUCCESS);
}

/* main() function */
int main(){

	int i;

	sem_t forks[philo_max];
	sem_t lock;
	
	// Signal catch
	signal(SIGINT, interrupt_handler);
	
	// Initialize the forks semaphore
	for(i = 0; i < philo_max; i++){
		sem_init(&forks[i], 0, 1);
	}

	/* Initialize lock such that only 4 philosophers
	   can hold a fork at a time */
	sem_init(&lock, 0, 4);

	// Initialize the philosopher threads
	for(i = 0; i < philo_max; i++){
		philo_struct *temp = malloc(sizeof(philo_struct));
		temp->name = i;			// assign name
		temp->forks = forks;	// point to forks semaphore array
		temp->lock = &lock;		// point to lock semaphore value
		temp->eat_count = 0;	// initialize eat_count to 0
		
		printf("Creating Philosopher %d\n", i+1);
		
		// Create pthread with philosopher information.
		pthread_create(&philosophers[i], NULL, philo_loop, (void *)temp);
	}
	
	// Join pthreads
	for(i = 0; i < philo_max; i++){
		pthread_join(philosophers[i], NULL);
	}
	
	return EXIT_SUCCESS;
}


