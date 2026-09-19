#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define DIMX (920)
#define DIMY (920)

#define max(X, Y) ((X) > (Y) ? (X) : (Y))

unsigned int P[max(DIMX, DIMY)][max(DIMX, DIMY)];
unsigned int TP[max(DIMX, DIMY)][max(DIMX, DIMY)];
unsigned int RRP[max(DIMX, DIMY)][max(DIMX, DIMY)];
unsigned int RLP[max(DIMX, DIMY)][max(DIMX, DIMY)];

void zeroIntMat(unsigned int Mat[][max(DIMX, DIMY)]);
void fillIntMat(unsigned int Mat[][max(DIMX, DIMY)]);
void printIntMat(unsigned int Mat[][max(DIMX, DIMY)]);

void transposeIntMat(unsigned int Mat[][max(DIMX, DIMY)], unsigned int TMat[][max(DIMX, DIMY)]);
void swapColIntMat(unsigned int Mat[][max(DIMX, DIMY)], unsigned int TMat[][max(DIMX, DIMY)]);
void swapRowIntMat(unsigned int Mat[][max(DIMX, DIMY)], unsigned int TMat[][max(DIMX, DIMY)]);



/* Read one header token, skipping comments only BEFORE the token.
 * Consume exactly its delimiter: raster bytes may themselves be whitespace
 * or '#', so never scan for the next non-whitespace byte after maxval. */
static int headerToken(FILE *in, char *token, size_t capacity)
{
    int ch;
    size_t length = 0;
    do {
        ch = fgetc(in);
        if(ch == '#')
            while(ch != '\n' && ch != EOF) ch = fgetc(in);
    } while(ch != EOF && isspace((unsigned char)ch));

    while(ch != EOF && !isspace((unsigned char)ch)) {
        if(length + 1 >= capacity) return 0;
        token[length++] = (char)ch;
        ch = fgetc(in);
    }
    token[length] = '\0';
    return length != 0 && ch != EOF;
}

static int headerNumber(FILE *in, unsigned int *value)
{
    char token[32];
    unsigned int number = 0;
    size_t i;
    if(!headerToken(in, token, sizeof(token))) return 0;
    for(i = 0; token[i]; ++i) {
        if(token[i] < '0' || token[i] > '9') return 0;
        number = number * 10 + (unsigned int)(token[i] - '0');
        if(number > 65535) return 0;
    }
    *value = number;
    return number != 0;
}

static int readPGM(FILE *in, unsigned int *maxval)
{
    char magic[8];
    unsigned int width, height, row, col;
    unsigned char pixels[DIMX];
    if(!headerToken(in, magic, sizeof(magic)) || strcmp(magic, "P5") ||
       !headerNumber(in, &width) || !headerNumber(in, &height) ||
       !headerNumber(in, maxval) || width > DIMX || height > DIMY ||
       *maxval > 255) {
        fprintf(stderr, "Expected an 8-bit P5 PGM no larger than 920x920.\n");
        return 0;
    }
    zeroIntMat(P);
    for(row = 0; row < height; ++row) {
        if(fread(pixels, 1, width, in) != width) {
            fprintf(stderr, "Incomplete PGM image data.\n");
            return 0;
        }
        for(col = 0; col < width; ++col) {
            if(pixels[col] > *maxval) {
                fprintf(stderr, "PGM pixel exceeds maxval.\n");
                return 0;
            }
            P[row][col] = pixels[col];
        }
    }
    return 1;
}

static int writePGM(FILE *out, unsigned int Mat[][max(DIMX, DIMY)],
                    unsigned int maxval)
{
    unsigned char pixels[DIMX];
    int row, col;
    if(fprintf(out, "P5\n%d %d\n%u\n", DIMX, DIMY, maxval) < 0) return 0;
    for(row = 0; row < DIMY; ++row) {
        for(col = 0; col < DIMX; ++col)
            pixels[col] = (unsigned char)Mat[row][col];
        if(fwrite(pixels, 1, DIMX, out) != DIMX) return 0;
    }
    return 1;
}

/* The repository uses rank + suit filenames, e.g. 10C.pgm and AH.pgm. */
static int cardSuit(const char *path)
{
    const char *name = path, *p, *dot;
    for(p = path; *p; ++p)
        if(*p == '/' || *p == '\\') name = p + 1;
    dot = strrchr(name, '.');
    if(!dot || dot == name || strlen(dot) != 4 ||
       tolower((unsigned char)dot[1]) != 'p' ||
       tolower((unsigned char)dot[2]) != 'g' ||
       tolower((unsigned char)dot[3]) != 'm') return 0;
    return toupper((unsigned char)dot[-1]);
}

int main(int argc, char *argv[])
{
    FILE *in, *out;
    unsigned int maxval;
    int suit, ok;
    unsigned int (*rotated)[max(DIMX, DIMY)];

    if(argc != 3) {
        fprintf(stderr, "Use: matrotate <inputfile> <outputfile>\n");
        return EXIT_FAILURE;
    }
    suit = cardSuit(argv[1]);
    if(suit != 'C' && suit != 'S' && suit != 'H' && suit != 'D') {
        fprintf(stderr, "Input filename must end in C.pgm, S.pgm, H.pgm or D.pgm.\n");
        return EXIT_FAILURE;
    }
    if(strcmp(argv[1], argv[2]) == 0) {
        fprintf(stderr, "Use a separate output filename.\n");
        return EXIT_FAILURE;
    }
    in = fopen(argv[1], "rb");
    if(!in) { perror(argv[1]); return EXIT_FAILURE; }
    ok = readPGM(in, &maxval);
    if(fclose(in) != 0) ok = 0;
    if(!ok) return EXIT_FAILURE;

    transposeIntMat(P, TP);
    if(suit == 'C' || suit == 'S') {
        swapColIntMat(TP, RRP);
        rotated = RRP;
    } else {
        swapRowIntMat(TP, RLP);
        rotated = RLP;
    }

    out = fopen(argv[2], "wb");
    if(!out) { perror(argv[2]); return EXIT_FAILURE; }
    ok = writePGM(out, rotated, maxval);
    if(fclose(out) != 0) ok = 0;
    if(!ok) {
        fprintf(stderr, "Could not write complete PGM: %s\n", argv[2]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


void swapRowIntMat(unsigned int Mat[][max(DIMX, DIMY)], unsigned int TMat[][max(DIMX, DIMY)])
{
    int idx, jdx;

    for(idx=0; idx<max(DIMX, DIMY); idx++)       
        for(jdx=0; jdx<max(DIMX, DIMY); jdx++)  
        {
            // copy into TMat and swap row values         
            TMat[idx][jdx]=Mat[max(DIMX, DIMY)-1-idx][jdx];
        }
}


void swapColIntMat(unsigned int Mat[][max(DIMX, DIMY)], unsigned int TMat[][max(DIMX, DIMY)])
{
    int idx, jdx;

    for(idx=0; idx<max(DIMX, DIMY); idx++)       
        for(jdx=0; jdx<max(DIMX, DIMY); jdx++)  
        {
            // copy into TMat and swap column values         
            TMat[idx][jdx]=Mat[idx][max(DIMX, DIMY)-1-jdx];
        }
}


void fillIntMat(unsigned int Mat[][max(DIMX, DIMY)])
{
    int cnt=0, idx, jdx;

    for(idx=0; idx<max(DIMX, DIMY); idx++)       
        for(jdx=0; jdx<max(DIMX, DIMY); jdx++)
        {
            Mat[idx][jdx]=(unsigned int)cnt;
            cnt++;
        }
}


void zeroIntMat(unsigned int Mat[][max(DIMX, DIMY)])
{
    int idx, jdx;

    for(idx=0; idx<max(DIMX, DIMY); idx++)       
        for(jdx=0; jdx<max(DIMX, DIMY); jdx++)
        {
            Mat[idx][jdx]=0;
        }
}


void transposeIntMat(unsigned int Mat[][max(DIMX, DIMY)], unsigned int TMat[][max(DIMX, DIMY)])
{
    int idx, jdx;

    for(idx=0; idx<max(DIMX, DIMY); idx++)
        for(jdx=0; jdx<max(DIMX, DIMY); jdx++)
        {
            // transpose row as column
            TMat[jdx][idx]=Mat[idx][jdx];
        }
}


void printIntMat(unsigned int Mat[][max(DIMX, DIMY)])
{
    int idx, jdx;

    for(idx=0; idx<max(DIMX, DIMY); idx++)
    {
         printf("\n");
         for(jdx=0; jdx<max(DIMX, DIMY); jdx++)
             printf("%02u ", Mat[idx][jdx]);
    }
    printf("\n\n");;
}

