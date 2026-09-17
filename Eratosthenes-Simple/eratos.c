#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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


#define NUM_THREADS (8)
//using bitmaps mean that each number corresponds to one bit instead of an int
//when indexing through range we divide index by CODE_LENGTH which is 8 corresponding to one byte to find the bit position
#define CODE_LENGTH ((sizeof(unsigned char))*8ULL)

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
        #pragma omp atomic update
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
        //printf("isprime=%d\n", primechk); 
    }

    // will all be TRUE here or 0xFF
    //print_isprime();
*/

    //goes through each prime number and eliminates it multiples  
    //only need to sieve using prime numbers up to the sqrt of max

    while( (p*p) <=  MAX)
    {
        //Give each thread a range of bits to prevent race conditions 
        unsigned long long total_bytes = MAX/CODE_LENGTH+1;
        unsigned long long range = total_bytes / NUM_THREADS; 

        #pragma omp num_threads(NUM_THREADS) 
        {
            //calculated exact start and end point for each thread based on thread id 
            unsigned int Threadidx = omp_get_thread_num(); 
            unsigned long long bit_start = Threadidxidx * range;
            unsigned long long bit_end = bit_start + range;

            //prevents last thread from going out of range
            if(bit_end > total_bytes)
            {
                bit_end = total_bytes;
            }

            //convert bits to byte position in isprime[]
            unsigned long long byte_start = bit_start * CODE_LENGTH;
            unsigned long long byte_end = bit_end * CODE_LENGTH -1;

            unsigned long long first_multiple = 

            //set all multiples of p in thread range to non prime 
            for (unsigned long long j = first; j <= byte_end; j += p)
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
    printf("\nNumber of primes [0..%llu]=%u\n\n", MAX, cnt);
    //print the first prime found in range while iterating backwards
    for (unsigned long long i = MAX; i >= 2; i--)
    {
        if (chk_isprime(i))
        {
            printf("largest prime: %llu\n",i);
        }
    }
    //find the prime factors of a given Semi prime SP = p1 * p2
    unsigned long long SP;
    unsigned long long p1;
    unsigned long long p2;
    //factors are < square root of semi prime 
    unsigned long long SP_range = sqrt(SP)
    int found = 0;

#pragma omp parallel for num_threads(NUM_THREADS) shared(found,p1,p2)
    for(i = 0; i<SP_range; i++)
    {
        //if factor is found skip work
        if(found) continue;

        //if i is both prime and has no remainder when dividing the semi prime 
        if(chk_isprime(i) && SP%i == 0)
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
    printf("%llu: factored into %llu, %llu\n",SP,p1,p2);
    return 0;
}

