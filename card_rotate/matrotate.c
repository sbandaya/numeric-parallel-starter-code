#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <omp.h>
#include <time.h>
// This is the dimension of a playing card with a 3:4 Aspect Ratio (standing upright)
#define DIMX (690)
#define DIMY (920)
#define NUM_CARDS (52)
//picks the largest length/height between x an y to make matrix size
#define max(X, Y) ((X) > (Y) ? (X) : (Y))
#define SQDIM (max(DIMX, DIMY))
#define NUM_THREADS (4)
//updated to a three-dimensional array so mulitple threads can be processed simultaneously 
unsigned char P[NUM_CARDS][SQDIM][SQDIM];     // Pixel array of gray values
unsigned char TP[NUM_CARDS][SQDIM][SQDIM];    // Transpose of Pixel array
unsigned char RP[NUM_CARDS][SQDIM][SQDIM];    // Rotation 
char suits[NUM_CARDS];
//transpose equation
void transposePixMat(unsigned char Mat[][SQDIM], unsigned char TMat[][SQDIM]);
//turn right
void swapColPixMat(unsigned char Mat[][SQDIM], unsigned char TMat[][SQDIM], int square_size);
//turn left
void swapRowPixMat(unsigned char Mat[][SQDIM], unsigned char TMat[][SQDIM], int square_size);

// PGM file utilities with simple byte by byte I/O
void readPGMHeaderSimple(int fdin, char *header);
void readPGMDataFast(int fdin, unsigned char Mat[][SQDIM]);
void writePGMFastSquare(int fdout, char *header, unsigned char Mat[][SQDIM]);

int main(int argc, char *argv[])
{
    int fdin, fdout, rowIdx, colIdx, card = 0;
    char header[NUM_CARDS][80]; //card header for each card 
    char inputPath[NUM_CARDS][1024]; //holds the file location for each card
    char outputPath[NUM_CARDS][1024]; //holds the file output for each card
    double fstart, fnow;
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    fstart = (double)start.tv_sec  + (double)start.tv_nsec / 1000000000.0;

    //modified example of dirlist.c 
    struct dirent *dp;
    char *fullpath;
    const char *input_folder="./cards_3x4_pgm"; //directory holding original cards
    const char *output_folder="./rotated_pgm"; //directory holding rotated cards
    
    DIR *dir = opendir(input_folder); 
    if (dir == NULL) {
        perror(input_folder);
        return 1;
    }
    
    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
    printf("\nstart test at %lf\n", fnow-fstart);

    //sequentially find all input and output file paths
    while ((dp = readdir(dir)) != NULL)
    {

        size_t len = strlen(dp->d_name);
        
        //skip non pgm files 
        if (strcmp(dp->d_name + len - 4, ".pgm") != 0)
            continue;

        //add the current files name/path to the array 
        snprintf(inputPath[card], sizeof(inputPath[card]),"%s/%s", input_folder, dp->d_name);
        snprintf(outputPath[card], sizeof(outputPath[card]),"%s/%s", output_folder, dp->d_name);

        //char before .pgm
        suits[card] = dp->d_name[len - 5];
        card++;
    }
    closedir(dir);



//give each thread a range of cards to read into matrix P
#pragma omp parallel for num_threads(NUM_THREADS) 
    for (int card = 0; card < NUM_CARDS; card++)
    {
        int fdin = open(inputPath[card], O_RDONLY);

        readPGMHeaderSimple(fdin, header[card]);
        readPGMDataFast(fdin, P[card]);
        close(fdin);

        //update the pgm header dimensions to 920x920
        header[card][26]='9'; header[card][27]='2'; header[card][28]='0';
    }



//give each thread a range of cards to perform rotations
#pragma omp parallel for num_threads(NUM_THREADS)
    for (int card = 0; card < NUM_CARDS; card++)
    {
        transposePixMat(P[card], TP[card]);

        //if cards are clubs or spades rotate right
        if (suits[card] == 'C' || suits[card] == 'S')
        {
            swapColPixMat(TP[card], RP[card], SQDIM);
        }

        //if cards are hearts or diamonds rate left
        if(suits[card] == 'H' || suits[card] == 'D')
        {
            swapRowPixMat(TP[card], RP[card], SQDIM);
        }
    }

//give each thread a range of cards to ouput rotated data
#pragma omp parallel for num_threads(NUM_THREADS)
    for (int card = 0; card < NUM_CARDS; card++)
    {
        int fdout = open(outputPath[card],O_WRONLY | O_CREAT | O_TRUNC,0644);
        writePGMFastSquare(fdout, header[card], RP[card]);
    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (double)now.tv_sec  + (double)now.tv_nsec / 1000000000.0;
    printf("stop test at %lf\n", fnow-fstart);
    printf("Read and then write of unmodified or test PGM done\n");

}


void swapRowPixMat(unsigned char Mat[][SQDIM], unsigned char TMat[][SQDIM], int square_size)
{
    int idx, jdx;

    for(idx=0; idx<square_size; idx++)       
        for(jdx=0; jdx<square_size; jdx++)  
        {
            // copy into TMat and swap row values         
            TMat[idx][jdx]=Mat[square_size-1-idx][jdx];
        }
}


void swapColPixMat(unsigned char Mat[][SQDIM], unsigned char TMat[][SQDIM], int square_size)
{
    int idx, jdx;

    for(idx=0; idx<square_size; idx++)       
        for(jdx=0; jdx<square_size; jdx++)  
        {
            // copy into TMat and swap column values         
            TMat[idx][jdx]=Mat[idx][square_size-1-jdx];
        }
}


void zeroPixMat(unsigned char Mat[][SQDIM])
{
    int idx, jdx;

    for(idx=0; idx<SQDIM; idx++)       
        for(jdx=0; jdx<SQDIM; jdx++)
        {
            Mat[idx][jdx]=0;
        }
}

void transposePixMat(unsigned char Mat[][SQDIM], unsigned char TMat[][SQDIM])
{
    int idx, jdx;

    for(idx=0; idx<SQDIM; idx++)
        for(jdx=0; jdx<SQDIM; jdx++)
        {
            // transpose row as column
            TMat[jdx][idx]=Mat[idx][jdx];
        }
}

void printPixMat(unsigned char Mat[][SQDIM], int square_size)
{
    int idx, jdx;

    for(idx=0; idx<square_size; idx++)
    {
         printf("\n");
         for(jdx=0; jdx<square_size; jdx++)
             printf("%03d ", Mat[idx][jdx]);
    }
    printf("\n\n");;
}

void readPGMHeaderSimple(int fdin, char *header)
{
    int bytesRead, bytesLeft, bytesWritten;

    //printf("Reading PGM header here\n");

    // header on each card is 38 bytes
    bytesLeft=38;
    bytesRead=read(fdin, (void *)header, bytesLeft);

    if(bytesRead < bytesLeft)
        exit(-1);
    /*
    else
    {
        header[bytesRead] = '\0';
        printf("header=%s\n", header);
    }

    */
}

void readPGMDataFast(int fdin, unsigned char Mat[][SQDIM])
{
    int bytesRead, bytesLeft, bytesWritten;
    int rowIdx, colIdx;

    //printf("Reading PGM data here\n");

    // now read in all of the data
    bytesRead=0;

    // read in whole rows at a time to speed up
    for(rowIdx = 0; rowIdx < DIMY; rowIdx++)
    {
        bytesRead = read(fdin, &Mat[rowIdx][0], DIMX);
    }

}

void writePGMFastSquare(int fdout, char *header, unsigned char Mat[][SQDIM])
{
    int bytesRead, bytesLeft, bytesWritten;
    int rowIdx, colIdx;

    //printf("Would write out a header and data here\n");
    bytesLeft=38;

    bytesWritten=write(fdout, (void *)header, bytesLeft);
    
    //printf("wrote %d bytes for header\n", bytesWritten);

    // now write out all of the data
    bytesWritten=0;

    for(rowIdx = 0; rowIdx < SQDIM; rowIdx++)
    {
        bytesWritten=write(fdout, (void *)&Mat[rowIdx][0], SQDIM);
        if(bytesWritten < SQDIM)
        {
            printf("ERROR in write\n"); exit(-1);
        }
    }
}



