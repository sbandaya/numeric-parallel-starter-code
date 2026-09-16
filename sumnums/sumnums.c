#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define COUNT (6000000000ULL)
#define NUM_THREADS (10)

typedef struct
{
    unsigned long long threadIdx;
    unsigned long long start;
    unsigned long long end;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];
// Thread specific globals
unsigned long long gsum[NUM_THREADS];

void *sumThread(void *threadp)
{
    unsigned long long i, idx, start, end;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    start = threadParams->start;
    end = threadParams->end;
    idx = threadParams->threadIdx;
    
    for(i=start; i<=end; i++)
    {
        gsum[idx] = gsum[idx] + i;
    }
    //print final sum instead of every iteration
    printf("thread idx=%llu, gsum=%llu\n", idx, gsum[idx]);
}

int main (int argc, char *argv[])
{
   unsigned long long range=COUNT/NUM_THREADS, i = 0;
   unsigned long long gsumall=0; 
   double fstart, fnow;
   struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    fstart = (double)start.tv_sec  + (double)start.tv_nsec / 1000000000.0;
   // initialize gsum array to zero
   for(i=0; i<NUM_THREADS; i++)
       gsum[i]=0;

   printf("Each thread subrange is %llu\n", range);

    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
    printf("\nstart test at %lf\n", fnow-fstart);
   for(i=0; i<NUM_THREADS; i++)
   {
      threadParams[i].threadIdx=i;
      threadParams[i].start=(i*range);
      threadParams[i].end=((i*range)+range)-1;

      pthread_create(&threads[i], (void *)0, sumThread, (void *)&(threadParams[i]));
   }

   for(i=0; i<NUM_THREADS; i++)
     pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
    printf("stop test at %lf\n", fnow-fstart);

   // we add int COUNT here in the final reduction since each worker
   // summed up to n-1, so we have to add final value n
   for(i=0; i<NUM_THREADS; i++)
       gsumall+=gsum[i];

   gsumall+=COUNT;

  // printf("TEST COMPLETE: gsum[0]=%lld, gsum[1]=%lld, gsumall=%lld\n", 
    //      gsum[0], gsum[1], gsumall);

   // Verfiy that sum of thread indexed sums is (n*(n+1))/2
   printf("TEST COMPLETE: gsumall=%llu, [n[n+1]]/2=%llu\n", gsumall, (COUNT/2*(COUNT+1)));
}
