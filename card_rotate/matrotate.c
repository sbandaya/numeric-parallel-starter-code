#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

// This is the dimension of a playing card with a 3:4 Aspect Ratio (standing upright)
#define DIMX (690)
#define DIMY (920)
#define NUM_CARDS (52)
//picks the largest length/height between x an y to make matrix size
#define max(X, Y) ((X) > (Y) ? (X) : (Y))
#define SQDIM (max(DIMX, DIMY))

//updated to a three-dimensional array so mulitple threads can be processed simultaneously 
unsigned char P[NUM_CARDS][SQDIM][SQDIM];     // Pixel array of gray values
unsigned char TP[NUM_CARDS][SQDIM][SQDIM];    // Transpose of Pixel array
unsigned char RRP[NUM_CARDS][SQDIM][SQDIM];   // Rotation Right of Pixel array
unsigned char RLP[NUM_CARDS][SQDIM][SQDIM];   // Rotation Left of Pixel array
char headers[NUM_CARDS][80];
char suits[NUM_CARDS]
//matrix initialization
void zeroPixMat(unsigned char Mat[][SQDIM]);
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
    int fdin, fdout, rowIdx, colIdx;
    //int bytesRead, bytesLeft, bytesWritten;
    char header[80];
    char inputPath[1024];
    char outputPath[1024];
    //modified example of dirlist.c 
    struct dirent *dp;
    char *fullpath;
    const char *input_folder="./cards_3x4_pgm"; // Directory target on NFS volume
    const char *output_folder="./rotated_pgm"; // Directory target on NFS volume
    DIR *dir = opendir(input_folder); // Open the directory - dir contains a pointer to manage the dir
    if (dir == NULL) {
        perror(input_folder);
        return 1;
    }
    printf("\nUse of readdir to find all card file names in a directory\n");

    while (dp=readdir(dir)) // if dp is null, there's no more content to read
    {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;

        snprintf(inputPath, sizeof(inputPath),"%s/%s", input_folder, dp->d_name);
        snprintf(outputPath, sizeof(outputPath),"%s/%s", output_folder, dp->d_name);

        fdin = open(inputPath, O_RDONLY);
        if (fdin < 0) {
            perror(inputPath);
            exit(1);
        }

        fdout = open(outputPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fdout < 0) {
            perror(outputPath);
            exit(1);
        }

        // initialize Pixel array with all zeros
        zeroPixMat(P);

        // read in the PGM data here
        readPGMHeaderSimple(fdin, header);
        readPGMDataFast(fdin, P);
        //get the character preceeding the .pgm which denotes the suit
        char suit = dp->d_name[strlen(dp->d_name) - 5];
        close(fdin);

        //transpose current matrix
        transposePixMat(P,TP);
        // Update header to be square 920x920
        header[26]='9'; header[27]='2'; header[28]='0';
        //club or spade rotate right
        if(suit == 'C' || suit == 'S')
        {
            swapColPixMat(TP,RRP,SQDIM);
            writePGMFastSquare(fdout, header, RRP);
        }
        //heart or diamond rotate left
        else if (suit == 'H' || suit == 'D')
        {
            swapRowPixMat(TP,RLP,SQDIM);
            writePGMFastSquare(fdout, header, RLP);
        }
        close(fdout);

    }

    closedir(dir); // close the handle (pointer)
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
        bytesRead=read(fdin, (void *)&P[rowIdx][0], DIMX);
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



