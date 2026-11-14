#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdint.h>



//counts all primes from 0 - 2^32 Billion and factors any semi-prime <= 2^64
#define MAX (4294967296ULL)

// Set your CPU's L1 data cache size (in bytes) here
const int64_t L1D_CACHE_SIZE = 32768;

//stores data from segmented sieve function for output in main 
typedef struct 
{
    long long count; //number of primes in range
    unsigned long long largest; //largest prime in range
    unsigned long long p1, p2;// prime factors of large semiprime
    int found;//marks if factors were found
} sieve_data;

//exmaple code in c++ https://github.com/kimwalisch/primesieve/wiki/Segmented-sieve-of-Eratosthenes
//limit is the upper bound of the sieve = MAX
//SP is the semiprime to be factored at most a 64 bit integer
//no longer uses bitmap required in basic implementation lowering memory requirements
sieve_data segmented_sieve(int64_t limit, unsigned long long SP)
{
    // intialize sieve_data
    sieve_data sd = {0,0,0,0,0}; 

    //only primes up to sqrt(limit) needed for cross off
    int64_t sqrt_limit = (int64_t) sqrt((double) limit);

    //intialize segment size which must not be lower than the sqrt(limit)
    int64_t segment_size = (sqrt_limit > L1D_CACHE_SIZE) ? sqrt_limit : L1D_CACHE_SIZE;

    //account for 2 since sieve only looks at odd numbers
    int64_t count = (limit < 2) ? 0 : 1;

    // we sieve primes >= 3
    int64_t i = 3;
    int64_t n = 3;
    int64_t s = 3;

    //allocate memory for numbers/primes/multiples needed for the segment
    char *sieve = malloc((size_t) segment_size);
    char *is_prime = malloc((size_t) (sqrt_limit + 1));
    int64_t *primes = malloc((size_t) (sqrt_limit + 1) * sizeof(int64_t));
    int64_t *multiples = malloc((size_t) (sqrt_limit + 1) * sizeof(int64_t));
    size_t nprimes = 0;

    if (!sieve || !is_prime || !primes || !multiples)
    {
        perror("malloc");
        exit(-1);
    }

    //mark all numbers up to sqrt(limit) as prime
    memset(is_prime, 1, (size_t) (sqrt_limit + 1));

    //check for 2
    if (limit >= 2)
    {
        sd.largest = 2;      
        if (SP % 2 == 0)     
        {
            sd.p1 = 2;         
            sd.p2 = SP / 2;    
            sd.found = 1;      
        }
  }

    //marks off non primes per segment 
    for (int64_t low = 0; low <= limit; low += segment_size)
    {
        //mark all numbers in current segment as prime
        memset(sieve, 1, (size_t) segment_size);

        // current segment = [low, high]
        int64_t high = low + segment_size - 1;
        high = (high < limit) ? high : limit;

        // generate sieving primes using simple sieve of Eratosthenes
        for (; i * i <= high; i += 2)
        {
            if (is_prime[i])
            {
                for (int64_t j = i * i; j <= sqrt_limit; j += i)
                {
                    is_prime[j] = 0;
                }
            }
        }
        
        // initialize sieving primes for segmented sieve
        for (; s * s <= high; s += 2)
        {
            if (is_prime[s])
            {
                primes[nprimes] = s;
                multiples[nprimes] = s * s - low;
                nprimes++;
            }
        }

        // sieve the current segment
        for (size_t k = 0; k < nprimes; k++)
        {
            int64_t j = multiples[k];
            for (int64_t step = primes[k] * 2; j < segment_size; j += step)
            {
                sieve[j] = 0;
            }
            multiples[k] = j - segment_size;
        }

    
        for (; n <= high; n += 2)
        {
            if (sieve[n - low]) // n is a prime
            {
                //check for prime factors and update sieve_data
                count++;
                sd.largest = (unsigned long long) n;
                if(!sd.found && SP % (unsigned long long) n == 0)
                {
                    sd.p1 = (unsigned long long) n;
                    sd.p2 = SP / (unsigned long long) n; 
                    sd.found = 1; 
                }
            }
        }
    }
    sd.count = count;

    free(sieve);
    free(is_prime);
    free(primes);
    free(multiples);
    return sd;
}

int main(int argc, char *argv[])
{
    unsigned long long SP; // semiprime
    char *endptr;
    sieve_data sd;
    
    printf("max uint = %u\n", (0xFFFFFFFF));
    printf("max long long = %llu\n", (0xFFFFFFFFFFFFFFFFULL));

    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <semiprime>\n", argv[0]);
        exit(-1);
    }
    SP = strtoull(argv[1], &endptr, 10);

    double fstart, fnow;
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    fstart = (double)start.tv_sec  + (double)start.tv_nsec / 1000000000.0;
    
    sd = segmented_sieve((int64_t)MAX,SP);

    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
    printf("Sieve completed in:%lf\n", fnow-fstart);

    printf("\nNumber of primes [0..%llu]=%lld\n\n", MAX, sd.count);
    printf("largest prime: %llu\n", sd.largest);

    if(sd.found)
    {
        printf("%llu: factored into %llu, %llu\n", SP, sd.p1, sd.p2);
    }
    else
    {
        printf("No factors of %llu found in range\n", SP);
    }
    return 0;
}
