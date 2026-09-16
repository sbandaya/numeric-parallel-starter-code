#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define COUNT (6000000000ULL)
#define NUM_THREADS (10)
//printf("Thread %d summing range %d to %d\n", idx, start, end);
//printf("thread idx=%d, gsum=%lld\n", idx, gsum[idx]);

int main (int argc, char *argv[])
{
   unsigned long long i;
   //total sum reduced 
   unsigned long long sum;

   //info for each thread following pthread implentation
   unsigned long long gsum[NUM_THREADS];

   //initialize gsum values
   for(i = 0 ; i < NUM_THREADS ; i++)
   {
      gsum[i]= 0;
   }

   double fstart, fnow;
   struct timespec start, now;
   clock_gettime(CLOCK_MONOTONIC, &start);
   fstart = (double)start.tv_sec  + (double)start.tv_nsec / 1000000000.0;

   clock_gettime(CLOCK_MONOTONIC, &now);
   fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
   printf("\nstart test at %lf\n", fnow-fstart);

   //splits work evenly among threads makes sure each thread has individual i to avoid clashes
   #pragma omp parallel num_threads(NUM_THREADS) private(i)
   {
      int idx = omp_get_thread_num();

      #pragma omp for reduction(+:sum) nowait
      for(i=0; i<=COUNT; i++)
      {
         sum += i;
         gsum[idx] += i;
      }

      // each thread prints this itself, right when it finishes its chunk
      printf("Thread %d gsum=%llu\n",idx, gsum[idx]);
   }

   clock_gettime(CLOCK_MONOTONIC, &now);
   fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
   printf("stop test at %lf\n", fnow-fstart);

   // Verfiy that sum of thread indexed sums is (n*(n+1))/2
   printf("TEST COMPLETE: sum=%llu, [n[n+1]]/2=%llu\n", sum, (COUNT/2*(COUNT+1)));
}
