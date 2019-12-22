#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>
#include <clock.h>
#include <kern/time.h>
#include "prime_gen.h"
static struct semaphore *sem1;
static struct semaphore *alldone;

//for storing prime number range
unsigned long rangeL[PRIMEGENERATOR],rangeR[PRIMEGENERATOR];  
//for storing result of different thread
long int resultt[PRIMEGENERATOR]; 
//for statistics.... 
struct timespec wait_time[PRIMEGENERATOR],run_time[PRIMEGENERATOR],
		s_time[PRIMEGENERATOR],e_time[PRIMEGENERATOR],duration;

void prime_generator(void * arg, unsigned long th_id) {
	gettime(&s_time[th_id]);
	unsigned long int *args=arg;
	unsigned long int start = args[0]; // Initial number
	unsigned long size = args[1]; // range
	unsigned long int i;
	unsigned long int count = 0l; // keep the count for number of prime generated
	unsigned long int range=start+size*th_id; // actual range for this thread


	//generating prime number -- square root method
	for (i = start; i < range + size; i += 1) {
		if (i % 2 != 0) {
			int flag = 1;
			unsigned long int j;
			unsigned long int l=floorSqrt(i);
			for (j = 3; j < l; j += 2) {
				if (i % j == 0) {
					flag = 0;
					break;
				}
			}
			if (flag == 1) {
				count++;
			}
		}
	}
	
	P(sem1);
	//get end time
	gettime(&e_time[th_id]);
	
	rangeL[th_id] = range;
	rangeR[th_id] = range+size;
	resultt[th_id] = count;
	V(sem1);
	V(alldone);
}
int runPrimeGenerator(int nargs, char **args) {
	(void) nargs; /* avoid compiler warnings */
	(void) args;
	sem1=sem_create("printer", 1);
	alldone=sem_create("alldone", 0);
	unsigned long int i;
	int result;
	
	kprintf("Please wait for the threads to finish\nThis may take up to 30 secs...........\n");

	unsigned long int x=(unsigned long int)STARTNUMBER;
	unsigned long int size=(unsigned long int)SIZE;
	for (i=0; i<PRIMEGENERATOR; i++) {
		unsigned long int data[2];
		data[0]=x;
		data[1]=size;
		char str[80];
		strcpy(str, "");

		// giving each thread different name
		// useful for debug. 
		char ch_arr[10][10] = {
                         {'a','\0'},
                         {'b','\0'},
                         {'c','\0'},
						 {'d','\0'},
						 {'e','\0'},
						 {'f','\0'},
						 {'g','\0'},
						 {'h','\0'},
						 {'i','\0'},
						 {'j','\0'},
        };
		strcat(str,ch_arr[i]);

		/**
		 * Thread creator and runner --
		 * you need to modify thread_fork function and thread data structure
		 * to randomly assign the priority
		 */
		result = thread_fork2(str, NULL,prime_generator,(void*)data, i,wait_time,run_time);
		if (result) {
			panic("runPrimeGenerator: Prime Generator Thread thread_fork failed: %s\n",
					strerror(result));
		}

		
		
	}
	//wait while all generators finished
	for(i=0;i<PRIMEGENERATOR; i++){
		P(alldone);
	}

	for (int i = 0; i < PRIMEGENERATOR; i++)
		{
			timespec_sub(&e_time[i],&s_time[i],&duration);
			kprintf("S %d [%lu-%lu]:%ld took %llu.%09lu(%llu.%09lu-%llu.%09lu) secs\n", 
				i, rangeL[i],rangeR[i],resultt[i],
				(unsigned long long)duration.tv_sec,(unsigned long)duration.tv_nsec,
				(unsigned long long)e_time[i].tv_sec,(unsigned long)e_time[i].tv_nsec,
				(unsigned long long)s_time[i].tv_sec,(unsigned long)s_time[i].tv_nsec);
			kprintf("[Wait time -- Run Time] : [%llu.%09lu -- %llu.%09lu]\n",
				(unsigned long long)wait_time[i].tv_sec,(unsigned long) wait_time[i].tv_nsec,
				(unsigned long long)run_time[i].tv_sec,(unsigned long) run_time[i].tv_nsec);
			
		}
		
	return 0;
}

/**
 * Function for sqrt (int only)
 */
int floorSqrt(int x){
	// Base cases
	if (x == 0 || x == 1)
		return x;

	// Staring from 1, try all numbers until
	// i*i is greater than or equal to x.
	int i = 1, result = 1;
	while (result <= x)
	{
		i++;
		result = i * i;
	}
	return i - 1;
}
