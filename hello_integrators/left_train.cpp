#include <iostream>
#include <cmath>
#include <mpi.h>
#include <time.h>

#define RANGE (1800.0f) 
#define DX (0.001f)
using namespace std;

using namespace std;
float ex3_accel(float time);
float ex3_vel(float time);
float ex3_pos(float time);
//allows it to 
float left_riemann_sum(float a, float b, int n, float func(float)) 
{
    float sum = 0.0f;

    for (int idx = 0; idx < n; idx++) 
    {
        float x = a + idx * DX;
        float fx = func(x);
        // Add the value of the function at the left endpoint of each subinterval.
        sum += fx;
    }

    return DX * sum;
}

int main(int argc, char* argv[]) 
{

    int my_rank, comm_sz;
    //MPI initialization 
    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    const float a = 0.0f;
    const float b = RANGE;
    const float  n = (b-a)/DX; // total steps = stop - start / step size

    int local_n = n / comm_sz; // # of steps for each process

    //starting and stop points for each process
    float local_a = my_rank * local_n * DX;
    float local_b = local_a + local_n * DX;
    
    float total_velocity = 0.0f;
    float total_position = 0.0f;

    //added barrier here so that all processes would start at same time for timing
    MPI_Barrier(MPI_COMM_WORLD);

    double fstart, fnow;
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    fstart = (double)start.tv_sec  + (double)start.tv_nsec / 1000000000.0;

    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
    printf("\nstart test at %lf\n", fnow-fstart);

    float local_velocity = left_riemann_sum(local_a, local_b, local_n, ex3_accel);
    float local_position = left_riemann_sum(local_a, local_b, local_n, ex3_vel);

    printf("Rank %d: local_a=%f local_b=%f local_n=%d local_velocity=%f local_position=%f\n", my_rank, local_a, local_b, local_n, local_velocity, local_position);

    MPI_Reduce(&local_velocity,&total_velocity,1,MPI_FLOAT,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&local_position,&total_position,1,MPI_FLOAT,MPI_SUM,0,MPI_COMM_WORLD);

    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
    printf("stop test at %lf\n", fnow-fstart);
    
    if(my_rank == 0)
    {
        cout.precision(15);
        cout << "final velocity = " << total_velocity << endl;
        cout << "final position = " << total_position << endl;
    }


    MPI_Finalize();
    
    
    /* SEQUENTIAL
    float a = 0.0;
    float b = RANGE;
    int n = RANGE/DX;
    float velocity = left_riemann_sum(a,b,n,ex3_accel);
    float position = left_riemann_sum(a,b,n,ex3_vel);
    cout << "final velocity = " << velocity << endl;
    cout << "final position = " << position << endl;
    */

    return 0;
}


float ex3_accel(float time)
{
    // computation of time scale for 1800 seconds
    static float tscale=1800.0f/(2.0f*(float)M_PI);
    // determined such that acceleration will peak to result in translation of 122,000.0 meters
    //static float ascale=0.2365893166123;
    static float ascale=0.236589076381454f;

    return (sinf(time/tscale)*ascale);
}


// determined based on known anti-derivative of ex4_accel function
float ex3_vel(float time)
{
    // computation of time scale for 1800 seconds
    static float tscale=1800.0f/(2.0f*(float)M_PI);
    // determined such that velocity will peak to result in translation of 122,000.0 meters
    static float vscale=0.236589076381454f*1800.0f/(2.0f*(float)M_PI);

    return ((-cosf(time/tscale)+1.0f)*vscale);
}


// determined based on known anti-derivative of ex4_vel function
float ex3_pos(float time)
{
    // computation of time scale for 1800 seconds
    static float tscale=1800.0f/(2.0f*(float)M_PI);
    // determined such that velocity will peak to result in translation of 122,000.0 meters
    static float vscale=0.236589076381454f*1800.0f/(2.0f*(float)M_PI);

    return ((-tscale*(sinf(time/tscale)+time))*vscale);
}


