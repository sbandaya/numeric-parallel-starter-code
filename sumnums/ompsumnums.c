#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define COUNT (1000000ULL)

//printf("Thread %d summing range %d to %d\n", idx, start, end);
//printf("thread idx=%d, gsum=%lld\n", idx, gsum[idx]);

int main (int argc, char *argv[])
{
   unsigned long long thread_count = 10;
   unsigned long long i;
   unsigned long long sum = 0;
   double fstart, fnow;
   struct timespec start, now;
   clock_gettime(CLOCK_MONOTONIC, &start);
   fstart = (double)start.tv_sec  + (double)start.tv_nsec / 1000000000.0;

   clock_gettime(CLOCK_MONOTONIC, &now);
   fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
   printf("\nstart test at %lf\n", fnow-fstart);
   #pragma omp parallel for num_threads(thread_count) reduction(+:sum) private(i)
   for(i=0; i<=COUNT; i++)
   {
        sum+= i;
   }
   clock_gettime(CLOCK_MONOTONIC, &now);
   fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
   printf("stop test at %lf\n", fnow-fstart);

   // Verfiy that sum of thread indexed sums is (n*(n+1))/2
   printf("TEST COMPLETE: sum=%llu, [n[n+1]]/2=%llu\n", sum, (COUNT/2*(COUNT+1)));
}
