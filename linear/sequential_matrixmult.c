#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

//multiply an NxN matrix A by an NxN matrix of the same size B generating resulting NxN matrix C 
void matrix_mult(int N, double A[N][N], double B[N][N], double C[N][N])
{
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

//multiply an NxN matrix A by a N column vector B generating resulting column vector C
void column_mult(int N, double A[N][N], double B[N], double C[N])
{
       for(int row = 0; row < N; row++)
    {
        for(int col = 0 ; col < N; col++)
        {
            C[row] += A[row][col] * B[col];
        }
    }
}

void print_mat(int n, double M[n][n])
{
    for (int row = 0; row < n; row++) 
    {
        for (int col = 0; col < n; col++)
        {
            printf("%8.2f ", M[row][col]);
        }
        printf("\n");
    }
}

void print_vec(int n, double v[n])
{
    for (int row = 0; row < n; row++)
        printf("%8.2f\n", v[row]);
}

int main{
    int N;
    double A[N][N],B[N][N],C[N][N];

    for (int row = 0; row < n; row++)
    {
        for (int col = 0; col < n; col++)
        {
            C[row][col] = 0.0;
        }
    }
    matrix_mult(N,A,B,C);       
    return 0;
}