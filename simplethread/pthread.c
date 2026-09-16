#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

#define NUM_THREADS 8

typedef struct
{
    int threadIdx;
    int threadSum;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];


void *counterThread(void *threadp)
{
    int sum=0, i;
    threadParams_t *threadParams = (threadParams_t *)threadp;
    //calculate expected value of sum 1....n
    int n = (threadParams->threadIdx+1)*100;
    int expected = n*(n+1)/2;
    //loop updated so each thread will do (thread# + 1) * 100
    for(i=1; i <= ((threadParams->threadIdx)+1)*100; i++)
        sum=sum+i;
 
    printf("Thread idx=%d, sum[0...%d]=%d, expected value: %d\n", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum,
           expected
        );

    //sends sum of each thread to its parameters 
    threadParams->threadSum = sum;
    return((void *)0);
}


int main (int argc, char *argv[])
{
   int i;
   int sum = 0;

   for(i=0; i < NUM_THREADS; i++)
   {
       threadParams[i].threadIdx=i;
       pthread_create(&threads[i],   // pointer to thread descriptor
                      (void *)0,     // use default attributes
                      counterThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );

   }

   for(i=0;i<NUM_THREADS;i++)
   {
       pthread_join(threads[i], NULL);
       //all threads sums are accumulated into one sum
       sum+= threadParams[i].threadSum;
   }
   printf("Total sum: %d\n",sum);
   printf("TEST COMPLETE\n");
}
