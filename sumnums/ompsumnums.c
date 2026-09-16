#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define COUNT (1000000LL)

//printf("Thread %d summing range %d to %d\n", idx, start, end);
//printf("thread idx=%d, gsum=%lld\n", idx, gsum[idx]);

int main (int argc, char *argv[])
{
   int thread_count = 10;
   int i;
   long long sum = 0;
#pragma omp parallel for num_threads(thread_count) reduction(+:sum) private(i)
   for(i=0; i<=COUNT; i++)
   {
        sum+= i;
   }

  // printf("TEST COMPLETE: gsum[0]=%lld, gsum[1]=%lld, gsumall=%lld\n", 
    //      gsum[0], gsum[1], gsumall);

   // Verfiy that sum of thread indexed sums is (n*(n+1))/2
   printf("TEST COMPLETE: sum=%lld, [n[n+1]]/2=%lld\n", sum, (COUNT*(COUNT+1))/2);
}
