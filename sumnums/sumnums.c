#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define COUNT (1000000ULL)
#define NUM_THREADS (10)

typedef struct
{
    int threadIdx;
    int start;
    int end;
} threadParams_t;


// POSIX thread declarations and scheduling attributes
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];

// Thread specific globals
long long gsum[NUM_THREADS];

void *sumThread(void *threadp)
{
    int i, idx, start, end;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    start = threadParams->start;
    end = threadParams->end;
    idx = threadParams->threadIdx;
    //printf("Thread %d summing range %d to %d\n", idx, start, end);

    for(i=start; i<=end; i++)
    {
        gsum[idx] = gsum[idx] + i;
    }
    //print final sum instead of every iteration
    printf("thread idx=%d, gsum=%lld\n", idx, gsum[idx]);
}

int main (int argc, char *argv[])
{
   int range=COUNT/NUM_THREADS, i = 0;
   unsigned long long gsumall=0; 
    

   // initialize gsum array to zero
   for(i=0; i<NUM_THREADS; i++)
       gsum[i]=0;

   printf("Each thread subrange is %d\n", range);

   for(i=0; i<NUM_THREADS; i++)
   {
      threadParams[i].threadIdx=i;
      threadParams[i].start=(i*range);
      threadParams[i].end=((i*range)+range)-1;

      pthread_create(&threads[i], (void *)0, sumThread, (void *)&(threadParams[i]));
   }

   for(i=0; i<NUM_THREADS; i++)
     pthread_join(threads[i], NULL);

   // we add int COUNT here in the final reduction since each worker
   // summed up to n-1, so we have to add final value n
   for(i=0; i<NUM_THREADS; i++)
       gsumall+=gsum[i];

   gsumall+=COUNT;

  // printf("TEST COMPLETE: gsum[0]=%lld, gsum[1]=%lld, gsumall=%lld\n", 
    //      gsum[0], gsum[1], gsumall);

   // Verfiy that sum of thread indexed sums is (n*(n+1))/2
   printf("TEST COMPLETE: gsumall=%llu, [n[n+1]]/2=%llu\n", gsumall, (COUNT*(COUNT+1))/2);
}
