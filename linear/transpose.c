// From https://www.geeksforgeeks.org/c-transpose-matrix/
//
// Adapt as necessary for transpose of any matrix including input from
// Lintest*.dat
//
// C Program to find transpose 
// of a square matrix 
#include <stdio.h> 
#include <time.h>
#define N 8192
  
// This function stores transpose 
// of A[][] in B[][] 
void transpose(int A[][N], int B[][N]) 
{ 
    int i, j; 
    for (i = 0; i < N; i++) 
        for (j = 0; j < N; j++) 
            // Assigns the transpose of element A[j][i] to 
            // B[i][j] 
            B[i][j] = A[j][i]; 
} 


// These are made global to avoid stack overflow
int A[N][N] = { { 1, 2, 3, 4 }, 
                { 5, 6, 7, 8 }, 
                { 9, 0, 1, 2 }, 
                { 3, 4, 5, 6 } }; 
int B[N][N], i, j; 


// Driver code 
int main() 
{ 
   transpose(A,B);
   double fstart, fnow;
   struct timespec start, now;
   clock_gettime(CLOCK_MONOTONIC, &start);
   fstart = (double)start.tv_sec  + (double)start.tv_nsec / 1000000000.0;
   transpose(A, B); 
   clock_gettime(CLOCK_MONOTONIC, &now);
   fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
   printf("N=%d transpose time = %.6lf ms\n", N, (fnow - fstart) * 1000.0);
   return 0; 
}

