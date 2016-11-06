/***********************************************
* Concurrency 3 - Categorical Mutual Exclusion *
* Christian Armatas & Mihai Dan     		   *
* 11/06/2016             					   *
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

typedef struct Node{
		int data;
		struct Node *next;
} data_struct;


void *search(void *arg);
void *delete(void *arg);
void *insert(void *arg);
void print_list(data_struct *linked_list);


int main(){

}


void *search(void *arg){

}


void *delete(void *arg){

}


void *insert(void *arg){

}


void print_list(data_struct *linked_list){

}


/*

Searchers:
	-examine the list
	-they can execute concurrently with each other
	
Inserters:
	-add new items to the end of the list
	-insertions must be mutually exclusive
		-prevents two inserters from inserting 
		 new items at about the same time
	-one insert can proceed in parallel with any number of searches
	
Deleters: 
	-remove items from anywhere in the list
	-at most one deleter process can access the list at a time
	-deletion must be mutually exclusive with searches and insertions

*/

