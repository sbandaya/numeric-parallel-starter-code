#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h> 

#define NUM_THREADS (4)
//multiply an NxN matrix A by an NxN matrix of the same size B generating resulting NxN matrix C 
void matrix_mult(int N, double A[N][N], double B[N][N], double C[N][N])
{ 
#pragma omp parallel for num_threads(NUM_THREADS) collapse(2)
    for(int row = 0; row < N; row++)
    {
        for(int col = 0 ; col < N; col++)
        {
            for(int cell = 0; cell < N; cell++)
            {
                C[row][col] += A[row][cell] * B[cell][col];
            }
        }
    }
}

//multiply an NxN matrix A by a N column vector X generating resulting column vector Y
void column_mult(int N, double A[N][N], double X[N], double Y[N])
{
#pragma omp parallel for num_threads(NUM_THREADS)
    for(int row = 0; row < N; row++)
    {
        for(int col = 0 ; col < N; col++)
        {
            Y[row] += A[row][col] * X[col];
        }
    }
}

void print_mat(int N, double M[N][N])
{
    for (int row = 0; row < N; row++) 
    {
        for (int col = 0; col < N; col++)
        {
            printf("%8.2f ", M[row][col]);
        }
        printf("\n");
    }
}

void print_vec(int N, double v[N])
{
    for (int row = 0; row < N ; row++)
        printf("%8.2f\n", v[row]);
}

int main(){
    int N;
    printf("Enter dimension n: ");
    if (scanf("%d", &N) != 1 || N < 1) {
        printf("Invalid size.\n");
        return 1;
    }
 
    //allocate memory for matricies to support 1000x1000 memory
    double (*A)[N] = malloc(sizeof(double[N][N]));
    double (*B)[N] = malloc(sizeof(double[N][N]));
    double (*C)[N] = calloc(N, sizeof(double[N]));  
    double *x = malloc(N * sizeof(double));
    double *y = calloc(N, sizeof(double));      
 
    //zero out the resulting C matrix
    for (int row = 0; row < N; row++)
    {
        for (int col = 0; col < N; col++)
        {
            C[row][col] = 0.0;
        }
    }

    //zero out the resulting y vector
    for (int row = 0; row < N; row++)
    {
        y[row] = 0.0;  
    }
    
    printf("Enter matrix A (%d x %d), row by row:\n", N, N);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            scanf("%lf", &A[i][j]);
 
    printf("Enter matrix B (%d x %d), row by row:\n", N, N);
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            scanf("%lf", &B[i][j]);
 
    printf("Enter column vector x (%d values):\n", N);
    for (int i = 0; i < N; i++)
        scanf("%lf", &x[i]);

    double fstart, fnow;
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    fstart = (double)start.tv_sec  + (double)start.tv_nsec / 1000000000.0;

    matrix_mult(N, A, B, C);

    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
    /* for verification
    printf("\nA * B =\n");
    print_mat(N, C);
    */
    printf("Matrix multiplication time:%lf\n", fnow-fstart);

    clock_gettime(CLOCK_MONOTONIC, &start);
    fstart = (double)start.tv_sec  + (double)start.tv_nsec / 1000000000.0;

    column_mult(N, A, x, y);
    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
    /* for verification
    printf("\nA * x =\n");
    print_vec(N, y);
    */
    printf("Vector multiplication time:%lf\n", fnow-fstart);
    free(A); free(B); free(C); free(x); free(y);
    return 0;
}