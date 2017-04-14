//////////////////////////////////////////////////////////////
/*
* Programmed once
* author:hank zhang(StanwieCB)
* student number: 515030910216
* mail: hankzhangcb@gmail.com
* 2017-3-31
*/
//////////////////////////////////////////////////////////////
/*
* CS356 Project1 Problem4 Burger Buddies Problem
*
* 1. Cooks, Cashiers, and Customers are each modeled as a thread.
* 2. Cashiers sleep until a customer is present.
* 3. A Customer approaching a cashier can start the order
*    process.
* 4. A Customer cannot order until the cashier is ready.
* 5. Once the order is placed, a cashier has to get a burger from
*    the rack.
* 6. If a burger is not available, a cashier must wait until one is
*  	 made.
* 7. The cook will always make burgers and place them on the
* 	 rack.
* 8. The cook will wait if the rack is full.
* 9. There are NO synchronization constraints for a cashier
* 	 presenting food to the customer.
*/
//////////////////////////////////////////////////////////////

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
//#include <sys/sem.h>

#define COOK_TIME 1
#define CUSTOM_COME 3

int num_of_cook = 0;
int num_of_cashier = 0;
int num_of_customer = 0;
int rack_size = 0;
int num_of_burger_on_rack = 0;
int num_of_hamburger = 0;
int num_of_served_customer = 0;

sem_t cook_mutex;
sem_t cashier_mutex;
sem_t terminal_mutex;
sem_t customer_waiting;
sem_t burger_available;
sem_t rack_available;
sem_t serve_finish;

//cook thread function
void *cook_func(void *argv)
{
	int ind = *(int*)argv;
	int tmp;
	do
	{
		//make a burger
		sleep(COOK_TIME);
		sem_wait(&cook_mutex);
		tmp = num_of_hamburger++;
		//enough burger
		if(tmp >= num_of_customer)
		{
			//printf("\nCook[%d] return\n\n", ind);		//debug code
			//printf("\n%d\n\n", num_of_hamburger);		//debug code
			sem_post(&cook_mutex);
			return;
		}
		sem_post(&cook_mutex);

		sem_wait(&cook_mutex);
		tmp = num_of_burger_on_rack++;
		//wait if rack is full
		if (tmp >= rack_size)
			sem_wait(&rack_available);
		//burger available
		if (tmp == 0)
			sem_post(&burger_available);
		sem_post(&cook_mutex);

		//make a burger
		sem_wait(&terminal_mutex);
		printf("Cook[%d] makes a burger\n", ind);
		sem_post(&terminal_mutex);

	}while(1);
}

//cashier thread function
void *cashier_func(void *argv)
{
	int ind = *(int*)argv;
	int tmp;

	do
	{
		//cashiers serving customers is enough 
		sem_wait(&cashier_mutex);
		tmp = num_of_served_customer++;
		if(tmp >= num_of_customer)
		{
			//printf("\nCashier[%d] return\n\n", ind); 	 //debug code
			sem_post(&cashier_mutex);
			return;
		}
		sem_post(&cashier_mutex);

		//serve if there comes a customer
		sem_wait(&customer_waiting);

		//accept order
		sem_wait(&terminal_mutex);
		printf("Cashier[%d] accpets an order\n", ind);
		sem_post(&terminal_mutex);

		sem_wait(&cashier_mutex);
		//wait if no burger
		if (num_of_burger_on_rack == 0)
			sem_wait(&burger_available);
		//take a burger
		tmp = num_of_burger_on_rack--;
		//rack is not full
		if (tmp == rack_size)
			sem_post(&rack_available);
		sem_post(&cashier_mutex);

		//take burger to customer
		sem_wait(&terminal_mutex);
		printf("Cashier[%d] takes a burger to customer\n", ind);
		sem_post(&terminal_mutex);
		sem_post(&serve_finish);

	}while(1);
}

//customer thread function
void *customer_func(void *argv)
{
	sleep(rand() % CUSTOM_COME);
	int ind = *(int*)argv;

	//customer comes
	sem_wait(&terminal_mutex);
	printf("Customer[%d] comes\n", ind);
	sem_post(&terminal_mutex);

	//waiting for service
	sem_post(&customer_waiting);
	sem_wait(&serve_finish);
}

int main(int argc, char **argv)
{
	//argument number exception test
	if (argc != 5)
	{
		printf("number of arguments is not correct!\n");
		return -1;
	}

	int i;

	int *cook_ind;
	int *cashier_ind;
	int *customer_ind;

	pthread_t *cook_thread;
	pthread_t *cashier_thread;
	pthread_t *customer_thread;

	pthread_attr_t *cook_thread_attr;
	pthread_attr_t *cashier_thread_attr;
	pthread_attr_t *customer_thread_attr;

	num_of_cook = (int)strtol(argv[1], NULL, 10);
	num_of_cashier = (int)strtol(argv[2], NULL, 10);
	num_of_customer = (int)strtol(argv[3], NULL, 10);
	rack_size = (int)strtol(argv[4], NULL, 10);

	//argument value/type exception test
	if (num_of_cook <= 0)
	{
		printf("number of cooks invalid!\n");
		return -1;
	}
	if (num_of_cashier <= 0)
	{
		printf("number of cashiers invalid!\n");
		return -1;
	}
	if (num_of_customer <= 0)
	{
		printf("number of customers invalid!\n");
		return -1;
	}
	if (rack_size <= 0)
	{
		printf("rack size invalid!\n");
		return -1;
	}


	printf("**********Burger Buddies Problem simulation begins!**********\n");
	printf("**********cook:%d  cashier:%d  customer:%d  rack:%d***********\n", 
		num_of_cook, num_of_cashier, num_of_customer, rack_size);

	//semaphore initialization and exception tes
	if(sem_init(&cook_mutex, 0, 1) == -1)
	{
		printf("cook_mutex initialization fail!\n");
		return -1;;
	}
	if(sem_init(&cashier_mutex, 0, 1) == -1)
	{
		printf("cashier_mutex initialization fail!\n");
		return -1;;
	}
	if(sem_init(&terminal_mutex, 0, 1) == -1)
	{
		printf("terminal_mutex initialization fail!\n");
		return -1;;
	}
	if(sem_init(&customer_waiting, 0, 0) == -1)
	{
		printf("customer_waiting initialization fail!\n");
		return -1;;
	}
	if(sem_init(&burger_available, 0, 0) == -1)
	{
		printf("burger_available initialization fail!\n");
		return -1;;
	}
	if(sem_init(&rack_available, 0, 0) == -1)
	{
		printf("rack_available initialization fail!\n");
		return -1;;
	}
	if(sem_init(&serve_finish, 0, 0) == -1)
	{
		printf("serve_finish initialization fail!\n");
		return -1;;
	}

	cook_ind = malloc(num_of_cook * sizeof(int));
	cashier_ind = malloc(num_of_cashier * sizeof(int));
	customer_ind = malloc(num_of_customer * sizeof(int));

	cook_thread = malloc(num_of_cook * sizeof(pthread_t));
	cashier_thread = malloc(num_of_cashier * sizeof(pthread_t));
	customer_thread = malloc(num_of_customer * sizeof(pthread_t));

	cook_thread_attr = malloc(num_of_cook * sizeof(pthread_attr_t));
	cashier_thread_attr = malloc(num_of_cashier * sizeof(pthread_attr_t));
	customer_thread_attr = malloc(num_of_customer * sizeof(pthread_attr_t));

	for (i = 0; i < num_of_cook; i++)
	{
		pthread_attr_init(cook_thread_attr + i);
		cook_ind[i] = i + 1;
		pthread_create(cook_thread + i, cook_thread_attr + i, 
			cook_func, cook_ind + i);
	}
	for (i = 0; i < num_of_cashier; i++)
	{
		pthread_attr_init(cashier_thread_attr + i);
		cashier_ind[i] = i + 1;
		pthread_create(cashier_thread + i, cashier_thread_attr + i, 
			cashier_func, cashier_ind + i);
	}
	for (i = 0; i < num_of_customer; i++)
	{
		pthread_attr_init(customer_thread_attr + i);
		customer_ind[i] = i + 1;
		pthread_create(customer_thread + i, customer_thread_attr + i, 
			customer_func, customer_ind + i);
	}

	for (i = 0; i < num_of_cook; i++)
		pthread_join(*(cook_thread + i), NULL);

	for (i = 0; i < num_of_cashier; i++)
		pthread_join(*(cashier_thread + i), NULL);

	for (i = 0; i < num_of_customer; i++)
		pthread_join(*(customer_thread + i), NULL);

	printf("**********Burger Buddies Problem simulation finishes!**********\n");

	free(cook_ind);
	free(cashier_ind);
	free(customer_ind);

	free(cook_thread);
	free(cashier_thread);
	free(customer_thread);

	free(cook_thread_attr);
	free(cashier_thread_attr);
	free(customer_thread_attr);

	sem_destroy(&cook_mutex);
	sem_destroy(&cashier_mutex);
	sem_destroy(&terminal_mutex);
	sem_destroy(&customer_waiting);
	sem_destroy(&burger_available);
	sem_destroy(&rack_available);
	sem_destroy(&serve_finish);

	return 0;
}