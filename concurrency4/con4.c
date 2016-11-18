/***********************************************
* Concurrency 4 - The Barbershop Problem 	   *
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
#include "mt19937ar.h"

#define NUM_CUSTOMERS 20
#define NUM_CHAIRS 3

pthread_t customer[20];
pthread_t barber;

sem_t barber_chair;
sem_t barber_snooze;
sem_t wait_room;
sem_t barber_cape;

int count = 0;

void *barber_func(void *arg);
void *customer_func(void *arg);
void cut_hair();
void get_haircut(int num);

int main(){
	int i;
	
	/* Initialize the semaphores */
	sem_init(&barber_cape, 0, 0);
	sem_init(&barber_chair, 0, 1);
	sem_init(&wait_room, 0, NUM_CHAIRS);
	sem_init(&barber_snooze, 0, 0);
	
	/* Create ptrhead for the barber */
	pthread_create(&barber, NULL, barber_func, NULL);
	
	/* Create pthreads for the 10 customers */
	for(i = 0; i < NUM_CUSTOMERS; i++){
		/* Don't overwhelm the barber */
		sleep(3);
		pthread_create(&customer[i], NULL, customer_func, (void *)&i);
	}
	
	/* Join the barber thread */
	pthread_join(barber, NULL);
	
	/* Join the customer threads */
	for(i = 0; i < NUM_CUSTOMERS; i++){
		pthread_join(customer[i], NULL);
	}
	
	return 0;
}

void *barber_func(void *arg){
	while(1){
		/* This is a while loop, because the barber
		   is constantly either cutting hair or sleeping. */
		printf("[B] No customers. Barber sound asleep...\n");
		
		/* Wait for sooze semaphore */
		sem_wait(&barber_snooze);
		
		/* Once he gets woken up... */
		printf("[B] Barber is awake.\n");
		
		/* ...he cuts hair. */
		cut_hair();
	}
}

void *customer_func(void *arg1){
	/* Get the customer number from the argument */
	int customer_num = *(int *)arg1;
	printf("[C] Customer %d arrived at the barber shop.\n", customer_num);
	
	/* If the chairs are occupied... */
	if(count >= NUM_CHAIRS){
		/*... customer leaves. */
		printf("[C] All chairs are occupied. Customer %d is now leaving.\n", customer_num);
	}else{
		/* Else wait for a chair. */
		sem_wait(&wait_room);
		count++;
		
		/* Wait for barber to finish current haircut */
		sem_wait(&barber_chair);
		
		/* Liberate wait room chair */
		sem_post(&wait_room);
		count--;
		
		/* Send wake-up call to barber */
		sem_post(&barber_snooze);
		
		/* Get haircut */
		get_haircut(customer_num);
		
		/* Liberate chair */
		sem_post(&barber_chair);
	}
}

void cut_hair(){
	/* Generate a random time for haircut */
	int haircut_time = genrand_int32() %12;
	printf("[CH] The haircut will take %d seconds.\n", haircut_time);
	
	/* Cut hair for specified amount of time */
	sleep(haircut_time);
	
	/* Barber takes off his cape... */
	sem_post(&barber_cape);
}

void get_haircut(int num){
	printf("[H] Customer %d is getting a haircut.\n", num);
	
	/* Put on the cape */
	sem_wait(&barber_cape);
}