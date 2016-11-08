/***********************************************
* Concurrency 3 - Categorical Mutual Exclusion *
* Christian Armatas & Mihai Dan     		   *
* 11/06/2016             					   *
***********************************************/

/**********************************************************************************************
* The following website was used as a reference 											  *
* for the basic data structure functions append()											  *
* and add().																				  *
* 																							  *
* http://www.cprogramming.com/snippets/source-code/singly-linked-list-insert-remove-add-count *
**********************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "mt19937ar.h"

// Linked list struct.
struct node{
		int data;
		struct node *next;
} *head;

pthread_t searcher[3];
pthread_t deleter[3];
pthread_t inserter[3];

pthread_mutex_t insert_mutex;
pthread_mutex_t delete_mutex;

sem_t delete_insert_lock;
sem_t delete_search_lock;


void *search(void *arg);
void *delete(void *arg);
void *insert(void *arg);
void add(int num);
void append(int num);
void interrupt_handler();
void print_list(struct node *linked_list);


int main(){
	int i;
	
	// Signal catch
	signal(SIGINT, interrupt_handler);
	
	// Initialize mutexes.
	pthread_mutex_init(&insert_mutex, NULL);
    pthread_mutex_init(&delete_mutex, NULL);

	// Initilize semaphores.
    sem_init(&delete_insert_lock, 0, 1);
    sem_init(&delete_search_lock, 0, 1);

	// Create threads.
	for(i = 0; i < 3; i++){
		pthread_create(&searcher[i], NULL, search, NULL);
		pthread_create(&inserter[i], NULL, insert, NULL);
		pthread_create(&deleter[i], NULL, delete, NULL);
	}

	// Join threads.
	for(i = 0; i < 3; i++){
		pthread_join(inserter[i], NULL);
		pthread_join(searcher[i], NULL);
		pthread_join(deleter[i], NULL);
	}
}

/* Create a new linked list with value num. */
void add(int num){
    struct node *temp;
    temp=(struct node *)malloc(sizeof(struct node));
    temp->data=num;
	
    if(head== NULL){
		head=temp;
		head->next=NULL;
    }else{
		temp->next=head;
		head=temp;
    }
}

/* Add value num at the end of linked list. */
void append(int num){
    struct node *temp,*right;
    temp= (struct node *)malloc(sizeof(struct node));
    temp->data=num;
	
    right=(struct node *)head;
	
    while(right->next != NULL){
		right=right->next;
	}
	
    right->next =temp;
    right=temp;
    right->next=NULL;
}


/* searches for random number */
void *search(void *arg){
	int num;
	int flag = 1, count = 0;
	struct node *temp;
	
	while(1){
		// Generate random number.
		num = genrand_int32() %69;	
		
		// Wait for delete search lock
		sem_wait(&delete_search_lock);
		
		temp=head;
		
		printf("[S] Searcher will search for %d\n", num);

		// Check if list is empty, else find and print number.
		if(temp==NULL){
			printf("[S] The linked list is empty... Cannot find %d\n", num);
		}else{
			// Check to see if num is in list
			while(temp != NULL){
				if(temp->data != num){
					temp=temp->next;
					++count;
				}else{
					// We found the num
					printf("[S] We found %d\n at index #%d\n", num, count);
					print_list(head);
					flag = 0;
					break;
				}
			}
			
			// If number is not in the list...
			if (flag != 0){
				printf("[S] Cannot find %d in linked list\n", num);
				print_list(head);
			}
		}
		
		// Unlock the delete search lock
		sem_post(&delete_search_lock);
		
		sleep(3);
	}
}

/* Deletes random index */
void *delete(void *arg){
	struct node *temp, *prev;
	int count, del_val, i;
		
	while(1){
		// Wait for the two locks.
		sem_wait(&delete_insert_lock);
		sem_wait(&delete_search_lock);
		
		// Lock the delete mutex.
		pthread_mutex_lock(&delete_mutex);
		
		temp=head;
		
		// Get the size of the linked list.
		count = 0;
		while(temp != NULL){
			++count;
			temp = temp->next;
		}

		// Pick a random index.
		del_val = genrand_int32() % count;
		
		// Point to the right spot in the list.
		temp = head;
		for(i = 0; i < del_val; i++){
			prev = temp;
			temp = temp->next;
		}
		
		// Delete accordingly.
		if(temp==head){
			head=temp->next;
			free(temp);
			printf("[D] Head value at index %d deleted.\n", del_val);
			print_list(head);
		}else{
			prev->next=temp->next;
			free(temp);
			printf("[D] Value at index %d deleted.\n", del_val);
			print_list(head);
		}
		
		// Unlock the delete mutex
		pthread_mutex_unlock(&delete_mutex);
		
		// Unlock the insert and search semaphores
		sem_post(&delete_insert_lock);
        sem_post(&delete_search_lock);
		
		sleep(6);
	}
}

/* Inserts value at the end of the list */
void *insert(void *arg){
	int num;
	struct node *temp;
	
	while(1){
		// Wait for the delete insert lock semaphore
		sem_wait(&delete_insert_lock);
		
		// Lock the insert mutex.
		pthread_mutex_lock(&insert_mutex);
		temp=head;
		
		// If list is empty, generate a list with a random value.
		if(temp==NULL){
			num = genrand_int32() %69;	
			add(num);
			printf("[I] CREATED LINKED LIST with value %d\n", num);
		}else{
			// Create random num and check if its in list
			num = genrand_int32() %69;
			while(temp != NULL){
				if(temp->data != num){
					temp=temp->next;
				}else{
					// we already have this value...
					num = genrand_int32() %69;
					temp=head;
				}
			}
			
			// Find end of list...
			temp=head;
			while(temp->next!=NULL){
				temp=temp->next;
			}
			// And add item to end of linked list
			append(num);
			printf("[I] Added value %d to linked list.\n", num);
		}
		// Print list.
		print_list(head);
		
		// Unlock delete insert lock semaphore.
		sem_post(&delete_insert_lock);
		
		// Unlock the insert mutex.
        pthread_mutex_unlock(&insert_mutex);
		sleep(5);
	}
}

/* Print linked list */
void print_list(struct node *linked_list){
	int print_val;
	
	printf("Linked List: ");
	
	if(linked_list == NULL){
		printf("List currently empty\n");
	}else{
		while(linked_list != NULL){
			print_val = linked_list->data;
			printf("%d ", print_val);
			linked_list = linked_list->next;
		}
		printf("\n");
	}
}

/* interrupt_handler() function */
void interrupt_handler(){
	int i;
	
	for(i = 0; i < 3; i++){
		pthread_detach(inserter[i]);
		pthread_detach(searcher[i]);
		pthread_detach(deleter[i]);
	}

	exit(EXIT_SUCCESS);
}

