#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// For values between 1 second indexed data, use linear interpolation to determine profile value at any "t".
//
// Wikipedia - https://en.wikipedia.org/wiki/Linear_interpolation
//
// This assumes that the profile is "piecewise linear", which is accurate for linear or constant functions, and 
// reasonably accurate for most non-linear functions over small intervals of 1 second.
//
// For high frequency non-linear functions spline and other more advanced curve fitting methods could be used, but
// the piecewise linear assumption is sufficient for CSCI 551 for acceleration profiles given.
//
// The alternative to a look-up table with linear interpolation is direct modeling of a function, but this requires 
// knowledge of the profile function as a linear, polynomial, or transcendental function or combination there-of, and
// this may not be known.

//#include "ex3accel.h"
//#include "ex3.h"
#include "ex4.h"
//#include "ex6lin.h"
//#include "ex6nonlin.h"


// table look-up for acceleration profile given and velocity profile determined
double table_accel(int timeidx);
double table_vel(int timeidx);

// indirect generation of acceleration or velocity at any time with table interpolation
double faccel(double time);
double fvel(double time);

// direct generation of acceleration at any time with math library and arithmetic
double ex3_accel(double time);
double ex4_accel(double time);
double ex4_vel(double time);

// Create velocity and position profiles (tables) the same size as acceleration profile
double VelProfile[sizeof(DefaultProfile) / sizeof(double)];
double PosProfile[sizeof(DefaultProfile) / sizeof(double)];

// Implement methods of integration
double Local_Riemann(double a, double b, int n, double func(double));
double Local_Trap(double a, double b, int n, double func(double));


void main(int argc, char *argv[])
{
    int idx;
    double time, dt=1.0; // dt=1.0 is the default to match spreadsheet
    int tsize = (int)(sizeof(DefaultProfile) / sizeof(double));
    unsigned long integration_steps=tsize;
    int steps_per_idx=integration_steps/tsize;
    unsigned long istep=0;
    int thread_count=1;
    double AccelStep, VelStep, PosStep, PeakVel = 0;
    struct timespec start, end;
    double fstart, fend;

    printf("argc=%d, argv[0]=%s\n", argc, argv[0]);
    if(argc == 2)
    {
        sscanf(argv[1], "%d", &thread_count);
        printf("Use: trainprofile [thread_count] [dt]\n");
        printf("Will use default time profile and use thread_count=%d\n", thread_count);
    }
    else if(argc == 3) 
    {
        sscanf(argv[1], "%d", &thread_count);
        sscanf(argv[2], "%lf", &dt);
        integration_steps = (unsigned long) ((double)tsize / dt); //?
        steps_per_idx=integration_steps/tsize; //?
        printf("Use: trainprofile [thread_count] [dt]\n");
        printf("Will use default time profile and use thread_count=%d, with dt=%lf for %lu steps and %d steps per table entry\n", thread_count, dt, integration_steps, steps_per_idx);
    }
    else
    {
        printf("Use: trainprofile [thread_count] [dt]\n");
        printf("Will use default time profile\n");
    }

    // Verify the static initializer or loaded CSV array for size and values
    //
    // Test outputs here should match the original spreadsheet specificaiton for the acceleration
    // profile.
    //
    // This is just a simple spot-checking test of expected values
    //
    printf("Number of values in profile = %lu for 1801 expected\n", sizeof(DefaultProfile)/sizeof(double));

    // Left Riemann sum test to match spreadsheet with shared data approach
    printf("\n\nLeft Riemann sum test for table with %d elements\n", tsize);
    clock_gettime(CLOCK_MONOTONIC, &start);
    VelProfile[0] = 0.0; VelStep=0;
    PosProfile[0] = 0.0; PosStep=0;
    idx=0;


    // **** SIMPLE SEQUENTIAL SHARED DATA INTEGRATION
    //
    // Hard to speed up due to data dependencies.
    //
    for(istep=1; istep < integration_steps; istep++)
    {
	    // for Left Riemann take acceleration at current time and add to previous value
	    time = (double)istep * dt;

	    // Note that velocity can be determined from profile or from math model for acceleration
        AccelStep = faccel(time);
        VelStep = VelStep + (AccelStep * dt);
        if(VelStep > PeakVel)
        {
            PeakVel = VelStep;
        }
        if((istep % steps_per_idx) == 0) 
        {
            idx++;
            VelProfile[idx] = VelStep;
        }

	    // Note that position must be determined from velocity determined above
        PosStep = PosStep + (VelStep * dt);

        if((istep % steps_per_idx) == 0) 
        {
            PosProfile[idx] = PosStep;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    fstart=start.tv_sec + (start.tv_nsec / 1000000000.0);
    fend=end.tv_sec + (end.tv_nsec / 1000000000.0);

    printf("Train from table in %lf seconds with %d samples: final velocity = %lf, final position = %lf, Peak velocity = %lf\n", 
	       (fend-fstart), tsize, VelProfile[tsize-1], PosProfile[tsize-1],PeakVel);

    exit(0);
}

// Simple look-up in accleration profile array
//
// Added array bounds check for known size of train arrays
//
double table_accel(int timeidx)
{
    long unsigned int tsize = sizeof(DefaultProfile) / sizeof(double);

    // Check array bounds for look-up table
    if(timeidx > tsize)
    {
        printf("timeidx=%d exceeds table size = %lu and range %d to %lu\n", timeidx, tsize, 0, tsize-1);
        exit(-1);
    }

    return DefaultProfile[timeidx];
}


// Simple linear interpolation example for table_accel(t) for any floating point t value
// for a table of accelerations that are 1 second apart in time, evenly spaced in time.
//
// accel[timeidx] <= accel[time] < accel[timeidx_next]
//
//
double faccel(double time)
{
    // The timeidx is an index into the known acceleration profile at a time <= time of interest passed in
    //
    // Note that conversion to integer truncates double to next lowest integer value or floor(time)
    //
    int timeidx = (int)time;

    // The timeidx_next is an index into the known acceleration profile at a time > time of interest passed in
    //
    // Note that the conversion to integer truncates double and the +1 is added for ceiling(time)
    //
    int timeidx_next = ((int)time)+1;

    // delta_t = time of interest - time at known value < time
    //
    // For more general case
    // double delta_t = (time - (double)((int)time)) / ((double)(timeidx_next - timeidx);
    //
    // If time in table is always 1 second apart, then we can simplify since (timeidx_next - timeidx) = 1.0 by definition here
    double delta_t = time - (double)((int)time);

    return ( 
               // The accel[time] is a linear value between accel[timeidx] and accel[timeidx_next]
               // 
               // The accel[time] is a value that can be determined by the slope of the interval and accel[timedix] 
               //
               // I.e. accel[time] = accel[timeidx] + ( (accel[timeidx_next] - accel[timeidx]) / ((double)(timeidx_next - timeidx)) ) * delta_t
               //
               //      ((double)(timeidx_next - timeidx)) = 1.0
               // 
               //      accel[time] = accel[timeidx] + (accel[timeidx_next] - accel[timeidx]) * delta_t
               //
               table_accel(timeidx) + ( (table_accel(timeidx_next) - table_accel(timeidx)) * delta_t)
           );
}

