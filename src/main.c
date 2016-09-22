/*
 * main.c
 *
 *  Producer/Consumer implementation using semaphores.
 *
 *  Created on: Sep 18, 2016
 *      Author: julio - julio.ppgca@gmail.com
 *
 *
 *      note: 	if you don't want any print messages, adjust the position of sem_post and
 *      		sem_wait of 's_wr_buf' only in 'g_buff' w/r operations.
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdint.h>

#define TEST_NUMBER	10000 	// how many loops at producer/consumer concurrent functions
#define BUFFER_SIZE	1	// choose the buffer size

uint32_t g_buff[BUFFER_SIZE]={}; //initialize buffer as global variable

sem_t s_prod, s_ctr_buf, s_wr_buf;	//item produced, buffer is not full, enabled w/r in buffer

void *t_prod(void *none);	// producer function prototype
void *t_cons(void *none);	// Consumer function prototype

int main()
{
	pthread_t tid1, tid2;	// thread identifications
	void *produced, *primes;

	sem_init(&s_prod,0,0);	// nothing produced yet
	sem_init(&s_ctr_buf,0,BUFFER_SIZE); //buffer is empty
	sem_init(&s_wr_buf,0,1); //ready to produce

	uint32_t i;
	for(i=0;i<TEST_NUMBER;i++) // let's start the test...
	{
		pthread_create(&tid2, NULL, t_cons, NULL);
		pthread_create(&tid1, NULL, t_prod, NULL);
	}
	pthread_join(tid1, &produced);
	pthread_join(tid2, &primes);

	printf("\n |********** Thats all folks! ***********| \n");
	printf("\t Items produced:\t %lu\n",(uint64_t)produced);
	printf("\t Prime numbers found:\t %lu\n",(uint64_t)primes);
	return 0;
}


void *t_prod(void *none)
{
	static uint32_t put=0; // index to put in the buffer
	static uint64_t produced=0;
	uint32_t prod,i;

	sem_wait(&s_ctr_buf); // buffer is not full? Proceed..
		sem_wait(&s_wr_buf); // there is no read operation going on..

			do{
				prod=0xff&random();
			}while(!(prod%2)); // get only odd numbers
			produced++;
			printf("Prod-> %d @ %d - odd: %s\n",prod,put,(prod%2)?"true":"false"); //print msg

			g_buff[put]=prod; // save in buffer at an empty position
			for(i=0;i<BUFFER_SIZE;i++) printf("\tBuf[%d]: %d\n",i,g_buff[i]); // print buffer
			if(++put>=BUFFER_SIZE) put=0; // circular type

		sem_post(&s_wr_buf); // release buffer w/r operation
	sem_post(&s_prod); // item produced and buffered.


	//usleep(500000); // for testing purposes
	return (void *)produced;
}

void *t_cons(void *none)
{
	static uint32_t get=0; // index to get from the buffer
	static uint64_t prime=0;
	uint32_t cons,i;

	sem_wait(&s_prod); // is there an item produced? Proceed.
		sem_wait(&s_wr_buf); // block buffer w/r

			cons=g_buff[get];
			for(i=2;i<=cons;i++)	if(!(cons%i)) break; // prime number test, i==cons: prime
			if(i==cons) prime++;
			g_buff[get]=0;
			printf("\t\t\t\t\t\tCons <- %d @ %d - prime: %s\n",cons,get,(i!=cons)?"false":"true");
			if(++get>=BUFFER_SIZE) get=0;
			for(i=0;i<BUFFER_SIZE;i++) printf("\t\t\t\t\t\t\tBuf[%d]: %d\n",i,g_buff[i]);

		sem_post(&s_wr_buf); // realease buffer w/r
	sem_post(&s_ctr_buf); // item consumed, free buffer position

	//usleep(500000); // for testing purposes
	return (void *)prime;
}
