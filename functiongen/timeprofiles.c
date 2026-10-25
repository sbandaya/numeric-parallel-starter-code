#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
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

#include "ex4.h"
// table look-up for acceleration profile given and velocity profile determined
double table_accel(int timeidx);
// indirect generation of acceleration or velocity at any time with table interpolation
double faccel(double time);

int main(int argc, char *argv[])
{
    int idx;
    double time, dt=1.0; // dt=1.0 is the default to match spreadsheet
    int tsize = (int)(sizeof(DefaultProfile) / sizeof(double));
    unsigned long integration_steps=tsize;
    int steps_per_idx=integration_steps/tsize;
    unsigned long istep=0;
    int thread_count=1;
    int start,end;
    double AccelStep, VelStep = 0, PosStep= 0, PeakVel = 0,FinalPos,FinalVel,local_PeakVel;
    double t_start, t_end, local_t, max_t;

    int my_rank, comm_sz;                                                  
    MPI_Init(&argc, &argv);                                              
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);                               
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);          

    printf("argc=%d, argv[0]=%s\n", argc, argv[0]);
    if(argc == 2)
    {
        sscanf(argv[1], "%d", &thread_count);
        if(my_rank == 0)
        {
            printf("Use: trainprofile [thread_count] [dt]\n");
            printf("Will use default time profile and use thread_count=%d\n", thread_count);
        }
    }
    else if(argc == 3) 
    {
        sscanf(argv[1], "%d", &thread_count);
        sscanf(argv[2], "%lf", &dt);
        if(my_rank == 0)
        {
            printf("Use: trainprofile [thread_count] [dt]\n");
            printf("Will use default time profile and use thread_count=%d, with dt=%lf for %lu steps and %d steps per table entry\n", thread_count, dt, integration_steps, steps_per_idx);
        }
    }
    else
    {
        if(my_rank == 0)
        {
            printf("Use: trainprofile [thread_count] [dt]\n");
            printf("Will use default time profile\n");
        }
    }

    // Verify the static initializer or loaded CSV array for size and values
    //
    // Test outputs here should match the original spreadsheet specificaiton for the acceleration
    // profile.
    //
    // This is just a simple spot-checking test of expected values
    //

    if(my_rank == 0)
    {
        printf("Number of values in profile = %lu for 1801 expected\n", sizeof(DefaultProfile)/sizeof(double));
        // Left Riemann sum test to match spreadsheet with shared data approach
        printf("\n\nLeft Riemann sum test for table with %d elements\n", tsize);
    }
    idx=0;
    integration_steps = (unsigned long) ((double)(tsize-1) / dt+0.5)/comm_sz; 
    start = my_rank * integration_steps;
    end = start + integration_steps;
    double *local_vel = malloc(integration_steps * sizeof(double)); 

    MPI_Barrier(MPI_COMM_WORLD);     
    t_start = MPI_Wtime();

    for(istep= start; istep < end; istep++)
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
        local_vel[istep - start] = VelStep;
    }

    double *total_velocity = NULL, *VelStart = NULL, local_VelStart;
    if(my_rank == 0)
    {
        total_velocity = malloc(comm_sz * sizeof(double));
        VelStart = malloc(comm_sz * sizeof(double));
    }
    MPI_Gather(&VelStep, 1, MPI_DOUBLE,  total_velocity, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
 
    if(my_rank == 0)
    {
        double sum = 0.0;
        for(idx = 0; idx < comm_sz; idx++)
        {
            VelStart[idx] = sum;
            sum = sum + total_velocity[idx];
        }
    }
    MPI_Scatter(VelStart, 1, MPI_DOUBLE, &local_VelStart, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    for(istep = start; istep < end; istep++)
    {
        PosStep = PosStep + ((local_vel[istep - start] + local_VelStart) * dt);
    }

    local_PeakVel = PeakVel + local_VelStart;
    MPI_Reduce(&PosStep, &FinalPos, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&VelStep, &FinalVel, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_PeakVel,&PeakVel,1, MPI_DOUBLE,MPI_MAX,0, MPI_COMM_WORLD);
    t_end = MPI_Wtime();
    local_t = t_end - t_start;
    MPI_Reduce(&local_t, &max_t, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if(my_rank == 0)
    {
        printf("Train from table in %lf seconds with %d samples: final velocity = %lf, final position = %lf, Peak velocity = %lf\n", 
	    max_t, tsize,FinalVel,FinalPos,PeakVel);
        free(total_velocity);
        free(VelStart);
    }
    free(local_vel);
    MPI_Finalize();

    return(0);
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
        MPI_Abort(MPI_COMM_WORLD, -1);
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

