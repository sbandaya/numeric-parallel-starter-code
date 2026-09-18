#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <time.h>
// Simple code to implement the original Eratosthenes Sieve
//
// Has been tested up to 1 billion using - https://primes.utm.edu/howmany.html
// to verify the expected # of primes in this range and each prime can be printed
// and compared with this list - http://compoasso.free.fr/primelistweb/page/prime/liste_online_en.php
//
// Should work as long as you have sufficent memory to malloc the bitmap.
//
// definte GIANT for range to 1 billion, otherwise range is to 1 million
//
#define GIANT

#ifdef GIANT
#define MAX (1000000000ULL)
#else
#define MAX (1000000ULL)
#endif


#define NUM_THREADS (4)
//using bitmaps mean that each number corresponds to one bit instead of an int
//when indexing through range we divide index by CODE_LENGTH which is 8 corresponding to one byte to find the bit position
#define CODE_LENGTH ((sizeof(unsigned char))*8ULL)

/* TEST CASES
1. 35: factored into 7, 5
2. 376223: factored into 439 & 857
3. 4006336753: factored into 46,411 & 86,323
4. 406615978649: factored into 470,303 & 864,583
5. 4154092115820191: factored into 47,868,193 & 86,781,887
6. 418155269059864129: factored into 481,346,903 & 868,719,143 
*/
//Semiprime 
#define SP (418155269059864129ULL)

// Static declaration replaced by malloc
//
//unsigned char isprime[(MAX/(CODE_LENGTH))+1];
unsigned char *isprime;

int chk_isprime(unsigned long long int i)
{
    unsigned long long int idx;
    unsigned int bitpos;

    idx = i/(CODE_LENGTH);
    bitpos = i % (CODE_LENGTH);

    //printf("i=%llu, idx=%llu, bitpos=%u\n", i, idx, bitpos);

    return(((isprime[idx]) & (1<<bitpos))>0);
}

int set_isprime(unsigned long long int i, unsigned char val)
{
    unsigned long long int idx;
    unsigned int bitpos;

    idx = i/(CODE_LENGTH);
    bitpos = i % (CODE_LENGTH);

    //printf("i=%llu, idx=%llu, bitpos=%u\n", i, idx, bitpos);

    if(val > 0)
    {
        //marks value at idx as prime by using bitwise or to change bit to a 1 
        isprime[idx] = isprime[idx] | (1<<bitpos);
    }
    else
    {
        isprime[idx] = isprime[idx] & (~(1<<bitpos));
    }

	return bitpos;
}


void print_isprime(void)
{
    long long int idx=0;

    printf("idx=%lld\n", (MAX/(CODE_LENGTH)));

    for(idx=(MAX/(CODE_LENGTH)); idx >= 0; idx--)
    {
        printf("idx=%lld, %02X\n", idx, isprime[idx]);
    }
    printf("\n");

}



int main(void)
{
    unsigned long long int i, j;
    unsigned long long int p=2;
    unsigned int cnt=0;
    unsigned long long int thread_idx=0;
	int idx=0, ridx=0, primechk;
    //find the prime factors of a given Semi prime SP = p1 * p2
    unsigned long long p1 = 0;
    unsigned long long p2 = 0;
    //factors are < square root of semi prime 
    unsigned long long SP_range = (unsigned long long)sqrt(SP);
    //flag for searching factors termination
    int found = 0;


    printf("max uint = %u\n", (0xFFFFFFFF));
    printf("max long long = %llu\n", (0xFFFFFFFFFFFFFFFFULL));

    if(!((isprime=malloc((size_t)(MAX/(CODE_LENGTH))+1)) > 0))
    {
        perror("malloc");
        printf("insufficient memory for prime sieve up to %lld\n", MAX);
        exit(-1);
    }
    else
    {
        printf("Sufficient memory for prime sieve up to %lld using %llu Mbytes at address=%p\n", 
               MAX, ((MAX/(CODE_LENGTH))+1)/(1024*1024), isprime);
    }

    // Not prime by definition
    // 0 & 1 not prime, 2 is prime, 3 is prime, assume others prime to start
    isprime[0]=0xFC; 

   double fstart, fnow;
   struct timespec start, now;
   clock_gettime(CLOCK_MONOTONIC, &start);
   fstart = (double)start.tv_sec  + (double)start.tv_nsec / 1000000000.0;

   clock_gettime(CLOCK_MONOTONIC, &now);
   fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
   printf("\nstart test at %lf\n", fnow-fstart);
//sets all numbers from 2 to max to be assumed prime 
//pragma is fine as all bits are being set to 1 so clashes are fine 
#pragma omp parallel for num_threads(NUM_THREADS)
    for(i=2; i<MAX; i++) 
    {
        set_isprime(i, 1); 
    }
  


/*
Checks if prime initialization worked correctly 
#pragma omp parallel for num_threads(NUM_THREADS)
    for(i=0; i<MAX; i++) 
    { 
        primechk = chk_isprime(i);
        printf("isprime=%d\n", primechk); 
    }

    // will all be TRUE here or 0xFF
    print_isprime();
*/

    //Give each thread a range of bits to prevent race conditions when parallelizing 
    unsigned long long total_bytes = MAX/CODE_LENGTH+1;
    unsigned long long range = (total_bytes + NUM_THREADS - 1) / NUM_THREADS;
    //goes through each prime number and eliminates it multiples  
    //only need to sieve using prime numbers up to the sqrt of max
    while( (p*p) <=  MAX)
    {
        #pragma omp parallel num_threads(NUM_THREADS) 
        {
            //calculated exact start and end point in array for each thread based on thread id 
            unsigned int ThreadIdx = omp_get_thread_num(); 
            unsigned long long start_idx = ThreadIdx * range;
            unsigned long long end_idx = start_idx + range;

            //prevents last thread from going out of range
            if(end_idx > total_bytes)
            {
                end_idx = total_bytes;
            }

            //convert index to individual numbers 
            unsigned long long start_num = start_idx * CODE_LENGTH;
            unsigned long long end_num = end_idx * CODE_LENGTH -1;

            unsigned long long first_multiple = ((start_num + p - 1) / p) * p;
            if (first_multiple < p*p)
            {
                first_multiple = p*p;
            }

            //set all multiples of p in thread range to non prime starting at p*p to avoid multiples checked by lower primes 
            for (unsigned long long j = first_multiple; j <= end_num; j += p)
            {
                set_isprime(j,0);
            }
        }

        // find next lowest prime - sequential process
        for(j=p+1; j<MAX+1; j++)
        {
            if(chk_isprime(j)) 
            { 
                p=j; 
                break;  // issue for speed-up with OpenMP
            }
        }

    }

//sums total number of primes after all non-primes have been 
#pragma omp parallel for num_threads(NUM_THREADS) reduction(+:cnt)
    for(i=0; i<MAX+1; i++)
    {
        if(chk_isprime(i))
        { 
            cnt++; 
            //printf("i=%llu\n", i); 
        }
    }

#pragma omp parallel for num_threads(NUM_THREADS) shared(found,p1,p2)
    for(i = 2; i<SP_range; i++)
    {
        //if factor is found skip work
        if(found) continue;

        //if i is both prime and has no remainder when dividing the semi prime 
        if(chk_isprime(i) && SP % i == 0)
        {
            //in case two threads find factors at same time prevent from accessing twice
            #pragma omp critical
            {
                //set flag
                found = 1;
                //current i is p1 and p2 is the number when you divide semiprime with other factor
                p1 = i;
                p2 = SP/p1;
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
    printf("stop test at %lf\n", fnow-fstart);

    printf("\nNumber of primes [0..%llu]=%u\n\n", MAX, cnt);
    //print the first prime found in range while iterating backwards
    for (unsigned long long i = MAX; i >= 2; i--)
    {
        if (chk_isprime(i))
        {
            printf("largest prime: %llu\n",i);
            break;
        }
    }

    if(found)
    {
        printf("%llu: factored into %llu, %llu\n",SP,p1,p2);
    }
    else
    {
        printf("No factors of %llu found in range\n", SP);
    }
    return 0;
}

