#include <iostream>
#include <cmath>
#include <mpi.h>
#include <time.h>

#define RANGE (1800) 
//#define DX(0.00001) //scaled example
#define DX (0.001)
using namespace std;

int thread_count=1;

using namespace std;
double ex3_accel(double time);
double ex3_vel(double time);
double ex3_pos(double time);
//allows it to 
double trapezoidal_rule(double a, double b, int n, double func(double)) 
{
    double sum = (func(a) + func(b)) / 2.0;
#pragma omp parallel for num_threads(thread_count) reduction(+:sum)
    for (int i = 1; i < n; i++) 
    {
        double x = a + i * DX;
        sum += func(x);
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

    const double a = 0.0;
    const double b = RANGE;
    const int n = (b-a)/DX; // total steps = stop - start / step size

    int local_n = n / comm_sz; // # of steps for each process

    //starting and stop points for each process
    double local_a = my_rank * local_n * DX;
    double local_b = local_a + local_n * DX;
    
    double fstart, fnow;
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    fstart = (double)start.tv_sec  + (double)start.tv_nsec / 1000000000.0;

    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
    printf("\nstart test at %lf\n", fnow-fstart);

    double total_velocity = 0;
    double total_position = 0;
    double local_velocity = trapezoidal_rule(local_a, local_b, local_n, ex3_accel);
    double local_position = trapezoidal_rule(local_a, local_b, local_n, ex3_vel);


    MPI_Reduce(&local_velocity,&total_velocity,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    MPI_Reduce(&local_position,&total_position,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);

    if(my_rank == 0)
    {
        cout.precision(7);
        cout << "final velocity = " << total_velocity << endl;
        cout << "final position = " << total_position << endl;
    }
    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
    printf("stop test at %lf\n", fnow-fstart);

    MPI_Finalize();
    
    
    /* SEQUENTIAL
    double a = 0.0;
    double b = RANGE;
    int n = RANGE/DX;
    double velocity = trapezoidal_rule(a,b,n,ex3_accel);
    double position = trapezoidal_rule(a,b,n,ex3_vel);
    cout << "final velocity = " << velocity << endl;
    cout << "final position = " << position << endl;
    */

    cout.precision(15);
    return 0;
}


double ex3_accel(double time)
{
    // computation of time scale for 1800 seconds
    static double tscale=1800.0/(2.0*M_PI);
    // determined such that acceleration will peak to result in translation of 122,000.0 meters
    //static double ascale=0.2365893166123;
    static double ascale=0.236589076381454;

    return (sin(time/tscale)*ascale);
}


// determined based on known anti-derivative of ex4_accel function
double ex3_vel(double time)
{
    // computation of time scale for 1800 seconds
    static double tscale=1800.0/(2.0*M_PI);
    // determined such that velocity will peak to result in translation of 122,000.0 meters
    static double vscale=0.236589076381454*1800.0/(2.0*M_PI);

    return ((-cos(time/tscale)+1)*vscale);
}


// determined based on known anti-derivative of ex4_vel function
double ex3_pos(double time)
{
    // computation of time scale for 1800 seconds
    static double tscale=1800.0/(2.0*M_PI);
    // determined such that velocity will peak to result in translation of 122,000.0 meters
    static double vscale=0.236589076381454*1800.0/(2.0*M_PI);

    return ((-tscale*(sin(time/tscale)+time))*vscale);
}


