#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
    unsigned long long SP_range = (unsigned long long)sqrt(SP);
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

    for(i=2; i<MAX; i++) 
    {
        set_isprime(i, 1); 
    }
  
    for(i=0; i<MAX; i++) 
    { 
        primechk = chk_isprime(i);
        //printf("isprime=%d\n", primechk); 
    }

    // will all be TRUE here or 0xFF
    //print_isprime();


    while( (p*p) <=  MAX)
    {
        //printf("p=%llu\n", p);

        // invalidate all multiples of lowest prime so far
        // 
        // simple to compose into a grid of invalidations
        //

        for(j=2*p; j<MAX+1; j+=p)
        {
            //printf("j=%llu\n", j);
            set_isprime(j,0);
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
