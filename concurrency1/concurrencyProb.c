#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "mt19937ar.h"
#define MAX 32

//	Prototypes
void* consumerFUNCTION(void *ptr);
void* producerFUNCTION(void *ptr);

// DataStruct	-	randNumbers
struct randNumbers {
	unsigned long val1;
	unsigned long val2;
};

struct randNumbers bufferContainer[32];
pthread_mutex_t mutexVar;
pthread_cond_t prod_cond, cons_cond;

//  This will check to see if we need to run rdrand or mtrand
int randomizerCheck(){
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;

	char vendor[13];
	
	eax = 0x01;

	__asm__ __volatile__(
	                     "cpuid;"
	                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
	                     : "a"(eax)
	                     );
	
	if(ecx & 0x40000000){
		//use rdrand
		return 2;
	}
	else{
		//use mt19937
		return 3;
	}
}

//	Initialize the buffer containing the data structs
void initializeBuffer(){
	int a;
	
    // Set bufferContainer values to 0
	for(a = 0; a < MAX; a++)
    {
    	bufferContainer[a].val1 = 0;
		bufferContainer[a].val2 = 0;    
    }
}


// PRODUCER FUNCTION
void* producerFUNCTION(void *ptr) {
	
	unsigned long randVal1, randVal2;
	unsigned long retLong, prodWait;
	char err;
	int i=0;
	
	// Initialize the tempStruct
	struct randNumbers tempStruct;
	tempStruct.val1 = 0;
	tempStruct.val2 = 0;
	
	//	Indefinite WHILE loop
	while(1){
		// Sleep for [3-7] seconds!
		retLong = randomizerCheck();
		if(retLong == 3){
			//use mt19937
			randVal1 = (genrand_int32()%7)+3;
		} else {
			//use rdrand
			__asm__ __volatile__(
				"rdrand %0 ; setc %1"
                : "=r" (randVal1), "=qm" (err)
			);
		}
		prodWait = randVal1;
		sleep(prodWait);
		
		// Create new struct and "add" it to the buffer
		retLong = randomizerCheck();
		if(retLong == 3){
			//use mt19937
			randVal1 = (genrand_int32()%8)+2;
			randVal2 = (genrand_int32()%8)+2;
		} else {
			//use rdrand
			__asm__ __volatile__(
				"rdrand %0 ; setc %1"
                : "=r" (randVal1), "=qm" (err)
			);
			__asm__ __volatile__("rdrand %0 ; setc %1"
                : "=r" (randVal2), "=qm" (err)
			);
		}
		tempStruct.val1 = randVal1; 	
		tempStruct.val2 = randVal2; 	
		
		pthread_mutex_lock(&mutexVar);	/* protect buffer */
		
		// If buffer is not empty, signal
		while (bufferContainer[i].val1 != 0){		       
			pthread_cond_wait(&prod_cond, &mutexVar);
		}
		
		pthread_mutex_unlock(&mutexVar);	/* release buffer */
		
		//	If buffer is empty or less than 32, keep producing
		if(bufferContainer[i].val1 == 0 || bufferContainer[i].val2 == 0){
			pthread_mutex_lock(&mutexVar);	/* protect buffer */
			printf("Produced: %lu, and now will sleep for %lu seconds\n", tempStruct.val1, prodWait);
			
			bufferContainer[i] = tempStruct;
			// inc if buffer isn't 32
			if(i != 32){			
				i += 1;
			} else {
				i = 0;
			}
		}
		// Wake up the consumer and release the buffer
		pthread_cond_signal(&cons_cond);
		pthread_mutex_unlock(&mutexVar);
	}
}


// CONSUMER FUNCTION
void* consumerFUNCTION(void *ptr) {
	
	int i=0;
	//	Indefinite WHILE loop
	while(1) {
		// Check to see if buffer is not empty
		if(!(bufferContainer[i].val1 == 0 || bufferContainer[i].val2 == 0)){
			pthread_mutex_lock(&mutexVar);	/* protect buffer */
		
			// If buffer is empty, signal
			while (bufferContainer[i].val1 == 0){
				pthread_cond_wait(&cons_cond, &mutexVar);
			}
			pthread_mutex_unlock(&mutexVar);	/* release the buffer */
			sleep(bufferContainer[i].val2);
			
			// "Consume" top struct on buffer  -	print off first value and sleep duration of second value
			printf("Consumer - Buffer value: %lu\n", bufferContainer[i].val1);
			printf("Consumer: Sleeping duration: %lu\n", bufferContainer[i].val2);
		
			pthread_mutex_lock(&mutexVar);	/* protect buffer */
			bufferContainer[i].val1 = 0;
			bufferContainer[i].val2 = 0;
			
			// If i isn't 32, keep incresing
			if (i != 32){
				i += 1;
			}
			else{
				i = 0;
			}
		
			pthread_cond_signal(&prod_cond);	/* wake up producer */
			pthread_mutex_unlock(&mutexVar);	/* release the buffer */
		}
	}
}


// MAIN
int main (int argc, char **argv){
	
	// Declare variables
	pthread_t producer, consumer;
	printf("\n\n     Starting up the producers and consumers...\n\n");
	initializeBuffer();
	
	// Initialize mutex, prod_cond, cons_cond
	pthread_mutex_init(&mutexVar, NULL);
	pthread_cond_init(&cons_cond, NULL);
	pthread_cond_init(&prod_cond, NULL);
	
	// Create the producer and the consumer threads
	pthread_create(&consumer, NULL, consumerFUNCTION, NULL);
	pthread_create(&producer, NULL, producerFUNCTION, NULL);
	
	// Wait for the threads to finish
	pthread_join(&consumer, NULL);
    pthread_join(&producer, NULL);
	
	// Cleanup
	pthread_mutex_destroy(&mutexVar);		/* Free up the_mutex */
    pthread_cond_destroy(&cons_cond);		/* Free up consumer condition variable */
    pthread_cond_destroy(&prod_cond);		/* Free up producer condition variable */
	
	printf("\n\n    Successfully ended function!   \n\n");
	
	return 0;
};
