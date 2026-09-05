// Sam Siewert, July 16, 2020
//
// Based on basic PSF convolution as documented in DSP Engineer's Handbook
//
// http://www.dspguide.com/pdfbook.htm
//
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <omp.h>

//#define IMG_HEIGHT (240)
//#define IMG_WIDTH (320)

#define IMG_HEIGHT (960)
#define IMG_WIDTH (1280)

// You should verify that this works correctly
// with you header comment length!
//
// Here it is hard-coded
#define HEADER_LENGTH (40)
//#define HEADER_LENGTH (22)

#define ITERATIONS (90)
//#define ITERATIONS (9000)

#define FAST_IO

typedef double FLOAT;

typedef unsigned int UINT32;
typedef unsigned long long int UINT64;
typedef unsigned char UINT8;

// PPM Edge Enhancement Code
//
UINT8 header[HEADER_LENGTH];
UINT8 R[IMG_HEIGHT*IMG_WIDTH];
UINT8 G[IMG_HEIGHT*IMG_WIDTH];
UINT8 B[IMG_HEIGHT*IMG_WIDTH];
UINT8 convR[IMG_HEIGHT*IMG_WIDTH];
UINT8 convG[IMG_HEIGHT*IMG_WIDTH];
UINT8 convB[IMG_HEIGHT*IMG_WIDTH];

// PPM image array with channels
UINT8 RGB[IMG_HEIGHT*IMG_WIDTH*3];

// controls sharpness
// increase from K=4.0 and F=8.0 for sharper edges
#define K 4.0
#define F 8.0
//#define F 80.0

FLOAT PSF[9] = {-K/F, -K/F, -K/F, -K/F, K+1.0, -K/F, -K/F, -K/F, -K/F};


//based on piseriesompfunct.c structure
void conv(void)
{
    int thread_count = omp_get_num_threads();
    int my_rank = omp_get_thread_num();

    int total_rows = (IMG_HEIGHT - 1) - 1;   
    int length = total_rows / thread_count;
    int residual = total_rows % thread_count;
    int iterations;

    int i; 
    int j;
    FLOAT outer = 0;
    FLOAT inner = 0;
    FLOAT temp;

    if (my_rank == (thread_count - 1))
    {
        iterations = length + residual;
    }
    else
    {
        iterations = length;
    }

    int start_row = 1 + my_rank * length;
    int end_row = start_row + iterations;

    for (i = start_row; i < end_row; i++)
    {
        for (j = 1; j < (IMG_WIDTH - 1); j++)
        {
            temp = 0;
            outer = (R[((i-1)*IMG_WIDTH)+j-1] + R[((i-1)*IMG_WIDTH)+j] + R[((i-1)*IMG_WIDTH)+j+1] + R[((i)*IMG_WIDTH)+j-1] + R[((i)*IMG_WIDTH)+j+1] + R[((i+1)*IMG_WIDTH)+j-1] + R[((i+1)*IMG_WIDTH)+j] + R[((i+1)*IMG_WIDTH)+j+1]) * PSF[0];            
            inner = (PSF[4] * R[((i)*IMG_WIDTH)+j]);
            temp = inner + outer;
            if(temp<0.0) temp=0.0;
            if(temp>255.0) temp=255.0;
            convR[(i*IMG_WIDTH)+j]=(UINT8)temp;

            temp = 0;
            outer = (G[((i-1)*IMG_WIDTH)+j-1] + G[((i-1)*IMG_WIDTH)+j] + G[((i-1)*IMG_WIDTH)+j+1] + G[((i)*IMG_WIDTH)+j-1] + G[((i)*IMG_WIDTH)+j+1] + G[((i+1)*IMG_WIDTH)+j-1] + G[((i+1)*IMG_WIDTH)+j] + G[((i+1)*IMG_WIDTH)+j+1]) * PSF[0];            
            inner = (PSF[4] * G[((i)*IMG_WIDTH)+j]);
            temp = inner + outer;
            if(temp<0.0) temp=0.0;
            if(temp>255.0) temp=255.0;
            convG[(i*IMG_WIDTH)+j]=(UINT8)temp;

            temp = 0;
            outer = (B[((i-1)*IMG_WIDTH)+j-1] + B[((i-1)*IMG_WIDTH)+j] + B[((i-1)*IMG_WIDTH)+j+1] + B[((i)*IMG_WIDTH)+j-1] + B[((i)*IMG_WIDTH)+j+1] + B[((i+1)*IMG_WIDTH)+j-1] + B[((i+1)*IMG_WIDTH)+j] + B[((i+1)*IMG_WIDTH)+j+1]) * PSF[0];            
            inner = (PSF[4] * B[((i)*IMG_WIDTH)+j]);
            temp = inner + outer;
            if(temp<0.0) temp=0.0;
            if(temp>255.0) temp=255.0;
            convB[(i*IMG_WIDTH)+j]=(UINT8)temp;
        }
    }
}

int main(int argc, char *argv[])
{
    int fdin, fdout, bytesRead=0, bytesWritten=0, bytesLeft, i, j, iter, rc, pixel, readcnt=0, writecnt=0;
    UINT64 microsecs=0, millisecs=0;
    FLOAT temp, fstart, fnow;
    struct timespec start, now;
    int thread_count= 4;

    clock_gettime(CLOCK_MONOTONIC, &start);
    fstart = (FLOAT)start.tv_sec  + (FLOAT)start.tv_nsec / 1000000000.0;
    
    if(argc < 3)
    {
       printf("Usage: sharpen input_file.ppm output_file.ppm\n");
       exit(-1);
    }
    else
    {
        if((fdin = open(argv[1], O_RDONLY, 0644)) < 0)
        {
            printf("Error opening %s\n", argv[1]);
        }
        //else
        //    printf("File opened successfully\n");

        if((fdout = open(argv[2], (O_RDWR | O_CREAT), 0666)) < 0)
        {
            printf("Error opening %s\n", argv[1]);
        }
        //else
        //    printf("Output file=%s opened successfully\n", "sharpen.ppm");
    }

    bytesLeft=HEADER_LENGTH-1;

    //printf("Reading header\n");

    // read in all data
    do
    {
        //printf("bytesRead=%d, bytesLeft=%d\n", bytesRead, bytesLeft);
        bytesRead=read(fdin, (void *)header, bytesLeft);
        bytesLeft -= bytesRead;
    } while(bytesLeft > 0);

    header[HEADER_LENGTH-1]='\0';

    printf("header = %s\n", header); 


#ifdef FAST_IO

    bytesRead=0;
    bytesLeft=IMG_HEIGHT*IMG_WIDTH*3;
    readcnt=0;
    printf("START: read %d, bytesRead=%d, bytesLeft=%d\n", readcnt, bytesRead, bytesLeft);

    // Read in RGB data in large chunks, requesting all and reading residual
    do
    {
        bytesRead=read(fdin, (void *)&RGB[bytesRead], bytesLeft);
        bytesLeft -= bytesRead;
        readcnt++;

        printf("read %d, bytesRead=%d, bytesLeft=%d\n", readcnt, bytesRead, bytesLeft);

    } while((bytesLeft > 0) && (readcnt < 3));

    printf("END: read %d, bytesRead=%d, bytesLeft=%d\n", readcnt, bytesRead, bytesLeft);

    // create in memory copy from input by channel
    for(i=0, pixel=0; i<IMG_HEIGHT*IMG_WIDTH; i++, pixel+=3)
    {
        R[i]=RGB[pixel+0]; convR[i]=R[i];
        G[i]=RGB[pixel+1]; convG[i]=G[i];
        B[i]=RGB[pixel+2]; convB[i]=B[i];
    }

#else

    // Read RGB data - Very slow one byte at time!
    for(i=0; i<IMG_HEIGHT*IMG_WIDTH; i++)
    {
        rc=read(fdin, (void *)&R[i], 1); convR[i]=R[i];
        rc=read(fdin, (void *)&G[i], 1); convG[i]=G[i];
        rc=read(fdin, (void *)&B[i], 1); convB[i]=B[i];
    }
#endif


    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (FLOAT)now.tv_sec  + (FLOAT)now.tv_nsec / 1000000000.0;
    printf("\nstart test at %lf\n", fnow-fstart);
    clock_gettime(CLOCK_MONOTONIC, &start);
    fstart = (FLOAT)start.tv_sec  + (FLOAT)start.tv_nsec / 1000000000.0;


    for(iter=0; iter < ITERATIONS; iter++)
    {
        #pragma omp parallel num_threads(thread_count)
            conv();

    }

    clock_gettime(CLOCK_MONOTONIC, &now);
    fnow = (FLOAT)now.tv_sec  + (FLOAT)now.tv_nsec / 1000000000.0;
    //printf("stop test at %lf for %d frames, fps=%lf\n\n", fnow-fstart, ITERATIONS, ITERATIONS/(fnow-fstart));
    printf("stop test at %lf for %d frames, fps=%lf, pps=%lf\n\n", fnow-fstart, ITERATIONS, ITERATIONS/(fnow-fstart), ((double)ITERATIONS*(double)IMG_HEIGHT*(double)IMG_WIDTH)/((double)(fnow-fstart)));

    rc=write(fdout, (void *)header, HEADER_LENGTH-1);

#ifdef FAST_IO

    // create in memory copy from input by channel
    for(i=0, pixel=0; i<IMG_HEIGHT*IMG_WIDTH; i++, pixel+=3)
    {
        RGB[pixel+0]=convR[i];
        RGB[pixel+1]=convG[i];
        RGB[pixel+2]=convB[i];
    }

    bytesWritten=0;
    bytesLeft=IMG_HEIGHT*IMG_WIDTH*3;
    writecnt=0;
    printf("START: write %d, bytesWritten=%d, bytesLeft=%d\n", writecnt, bytesWritten, bytesLeft);

    // Write RGB data in large chunks, requesting all at once and writing residual
    do
    {
        bytesWritten=write(fdout, (void *)&RGB[bytesWritten], bytesLeft);
        bytesLeft -= bytesWritten;
        writecnt++;

        printf("write %d, bytesWritten=%d, bytesLeft=%d\n", writecnt, bytesWritten, bytesLeft);

    } while((bytesLeft > 0) && (writecnt < 3));

    printf("END: write %d, bytesWritten=%d, bytesLeft=%d\n", writecnt, bytesWritten, bytesLeft);

#else
    // Write RGB data - very slow 1 byte at a time!
    for(i=0; i<IMG_HEIGHT*IMG_WIDTH; i++)
    {
        rc=write(fdout, (void *)&convR[i], 1);
        rc=write(fdout, (void *)&convG[i], 1);
        rc=write(fdout, (void *)&convB[i], 1);
    }
#endif


    close(fdin);
    close(fdout);
 
}
