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

