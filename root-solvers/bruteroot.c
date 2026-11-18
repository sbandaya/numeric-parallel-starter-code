// This is the simplest brute force stepping method possible to find a root
// on an interval with a fixed step size equal to error tolerance.
//
// It simply detects a sign change and declares the root to be half way between
// the step before the sign change and the step after and quits.
//
// Inspired by questions from Section 02 CSCI 551, 2022
//
// Why not just search by making small steps for a zero crossing.  This definitely works, but
// does require small step sizes, potentially many steps to search a large interval, and error
// will simply be stepsize / 2.0
//
// Makes an intersting comparison to Newton, Bisection, and Regula Falsi, all which base stepping
// on an estimate of where the root is on an interval.
//
// Generally, Newton should converge fastest, followed by Regula Falsi, then Bisection, and the Bruteroot will
// get the root stepping in one direction looking for a simple sign change, but is the slowest.
//
// This code has an added outter do-while loop to find all roots on a larger interval.
//
// Modified to handle "osculating" real roots - e.g., a parabola that "kisses" the X axis (intersects), but does
// not cross the X axis with a sign change.
//
#include <stdio.h>
#include<stdlib.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <omp.h>
#define NUM_THREADS 1
// First model a function with desmos.com for example to "see" roots
// 
// https://www.desmos.com/calculator
//
double f(double x)
{
    // Fill in your function here using the math library

    //return x*log10(x) - 1.2;
    return cos(x*x);
    //return cos(x);
    //return sin(x);
    //return sin(x*x);

    //return((x*x)+(2.0*x)+1);
    //return((x*x)+(2.0*x)+2);

    // Note this osculating parabola will break brute-root as there is no
    // sign change to detect - can we fix this?
    // Detect zero within tolerance, which here is < step
    //return((x*x)+(2.0*x)+1.0);

    // As can be seen with Desmos, this example equation has roots near: -2.851, 4.758, and 7.792
    // return (-(x*x*x) + 9.7*(x*x) -1.3*x -105.7);
}

int main(int argc, char * argv[])
{
    unsigned long long itr, maxitr, total_itr=0;
    unsigned int rootfound=0;
    double max_error=0.0, fstart=0.0, fstop=0.0;
    double step=2.0*DBL_EPSILON, local_start, x1, xfinal, sign_changed=0, start;
    struct timespec start_t, stop_t;


    printf("smallest step is 2x epsilon is %20.19lf", (double)(2.0* DBL_EPSILON));

    if (argc != 5)
    {
        printf("Usage: %s <local_start> <xfinal> <step> <max iterations>\n", argv[0]);
        exit(-1);
    }
    local_start = atof(argv[1]);
    xfinal = atof(argv[2]);
    step = atof(argv[3]);
    maxitr = atoi(argv[4]);
    start=local_start;

    printf("\nWill step at %20.19lf from %lf to %lf to find each root, with %lld total search steps\n\n",
           step, local_start, xfinal, (((unsigned)(xfinal-local_start)+1)*maxitr));
    printf("Starting search:\n");
    clock_gettime(CLOCK_MONOTONIC, &start_t);
    fstart = (double)start_t.tv_sec  + (double)start_t.tv_nsec / 1000000000.0;

    //total number of steps to be divided among threads replacing while loop
    unsigned long long total_steps = (unsigned long long)((xfinal - local_start) / step) +1;
    
    //divides total number of steps among threads summing total number of roots and finding max error among threads
    #pragma omp parallel for num_threads(NUM_THREADS) private(x1, sign_changed) reduction(+:rootfound) reduction(max:max_error)
    for (itr=0; itr < total_steps; itr++) // find next root on sub-interval
    {
        double local_start = start + itr*step;
        x1= local_start + step;

        // Any difference in sign between local_start and x1 will result in a sign_changed
        // less than zero - a crossing has been detected
        sign_changed = f(local_start) * f(x1);

        // X-axis Crossing ROOT case
        //
        if(sign_changed < 0.0)
        {
            //printf("\n*** Sign change at %lld iterations: estimated root at %16.15lf, f[root]=%16.15lf\n", total_itr+itr, (x1+local_start)/2.0, f((x1+local_start)/2.0));
            if(fabs(f((x1+local_start)/2.0)) > max_error) max_error=fabs(f((x1+local_start)/2.0));
            rootfound++;
        }

        // Osculating ROOT case
        //
        else if( fabs(f((x1+local_start)/2.0)) < step && !(fabs(f(local_start-step/2.0)) < step)) // hit zero within tolerance - special case for osculating functions
        {
            //printf("\n*** Osculating ZERO at %lld iterations: estimated DOUBLE root at %16.15lf, f[root]=%16.15lf\n",  total_itr+itr, (x1+local_start)/2.0, f((x1+local_start)/2.0));
            if(fabs(f((x1+local_start)/2.0)) > max_error) max_error=fabs(f((x1+local_start)/2.0));

            // Because by math rules, this must be a double root
            rootfound=rootfound+2;
        }

    }


    //matches output for original implementation
    total_itr = total_steps;
    local_start = start + total_steps*step;

    // Indicate searching progress
    printf(".");
    fflush(stdout);

    clock_gettime(CLOCK_MONOTONIC, &stop_t);
    fstop = (double)stop_t.tv_sec  + (double)stop_t.tv_nsec / 1000000000.0;
    printf("\n\nSearch Results (competed in %lf milliseconds):\n", (fstop-fstart)*1000.0);

    if(!rootfound)
    {
        printf("After %lld iterations: No solution (zero crossing) found on interval %lf to %lf\n", total_itr, start, local_start);
    }
    else
    {
        printf("%d roots found on interval %lf to %lf, with max error=%lf in %lld iterations\n",
               rootfound, start, local_start, max_error, total_itr);
    }

    return 1;
}
