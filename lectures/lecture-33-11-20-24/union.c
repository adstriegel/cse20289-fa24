// union.c

#include <stdio.h>
#include <string.h>

#define NUM_INTS    3

union IntBytes
{
    int     TheInts[NUM_INTS];
    char    TheBytes[NUM_INTS*sizeof(int)]; 
};

int main ()
{
    union IntBytes  Whaaat;

    for(int j=0; j<NUM_INTS; j++)
    {
        Whaaat.TheInts[j] = j;
    }

    printf("== Proper Outputs ==\n");

    for(int k=0; k<NUM_INTS*sizeof(int); k++)
    {
        printf("Hex at %d: 0x%02X\n", k, Whaaat.TheBytes[k]);
    }

    /* Let's go crazy */
    char * pBytes;
    pBytes = (char *) &Whaaat;

    printf("== l33t Style ==\n");

    for(int k=0; k<NUM_INTS*sizeof(int); k++)
    {
        printf("Hex at %d: 0x%02X\n", k, *(pBytes+k));
    }
}