#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
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

//allows for all semiprimes up to 64 bits
#ifdef GIANT
#define MAX (4294967296ULL)
#else
#define MAX (1000000ULL)
#endif

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

// Set your CPU's L1 data cache size (in bytes) here
const int64_t L1D_CACHE_SIZE = 32768;

//exmaple code in c++ https://github.com/kimwalisch/primesieve/wiki/Segmented-sieve-of-Eratosthenes
//modified Walisch's segmented sieve to c code including the helpers included in exercise 2
void segmented_sieve(int64_t limit)
{
  int64_t sqrt_limit = (int64_t) sqrt((double) limit);
  int64_t segment_size = (sqrt_limit > L1D_CACHE_SIZE) ? sqrt_limit : L1D_CACHE_SIZE;
  int64_t count = (limit < 2) ? 0 : 1;

  // we sieve primes >= 3
  int64_t i = 3;
  int64_t n = 3;
  int64_t s = 3;

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
  memset(is_prime, 1, (size_t) (sqrt_limit + 1));

  // 2 is the only even prime
  if (limit >= 2)
    set_isprime(2, 1);

  for (int64_t low = 0; low <= limit; low += segment_size)
  {
    memset(sieve, 1, (size_t) segment_size);

    // current segment = [low, high]
    int64_t high = low + segment_size - 1;
    high = (high < limit) ? high : limit;

    // generate sieving primes using simple sieve of Eratosthenes
    for (; i * i <= high; i += 2)
      if (is_prime[i])
        for (int64_t j = i * i; j <= sqrt_limit; j += i)
          is_prime[j] = 0;

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
        sieve[j] = 0;
      multiples[k] = j - segment_size;
    }

    for (; n <= high; n += 2)
      if (sieve[n - low]) // n is a prime
      {
        count++;
        set_isprime((unsigned long long) n, 1);
      }
  }

  printf("%lld primes found.\n", (long long) count);

  free(sieve);
  free(is_prime);
  free(primes);
  free(multiples);
}

int main(int argc, char *argv[])
{
    unsigned long long int i, j;
    unsigned long long int p=2;
    unsigned int cnt=0;
    unsigned long long int thread_idx=0;
	int idx=0, ridx=0, primechk;
    unsigned long long int SP;
    unsigned long long p1 = 0;
    unsigned long long p2 = 0;
    unsigned long long SP_range = 0;
    int found = 0;
    char *endptr;


    printf("max uint = %u\n", (0xFFFFFFFF));
    printf("max long long = %llu\n", (0xFFFFFFFFFFFFFFFFULL));

    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <semiprime>\n", argv[0]);
        exit(-1);
    }
    SP = strtoull(argv[1], &endptr, 10);
    SP_range = (unsigned long long)sqrt(SP);
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

    memset(isprime, 0, (size_t)(MAX/CODE_LENGTH)+1);
    segmented_sieve((int64_t) MAX);


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

     for(i = 2; i<SP_range; i++)
    {
        //if factor is found skip work
        if(found) continue;

        //if i is both prime and has no remainder when dividing the semi prime 
        if(chk_isprime(i) && SP % i == 0)
        {
            //set flag
            found = 1;
            //current i is p1 and p2 is the number when you divide semiprime with other factor
            p1 = i;
            p2 = SP/p1;
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
