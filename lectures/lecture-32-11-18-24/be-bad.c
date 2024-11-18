/* be-bad.c : Turn to the dark side */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Our example from class */
size_t string_length(const char *s) 
{    
    const char *c; 

    for (c = s; *c; c++);     
    return (c - s); 
}

/* Github Copilot - Generate the code plz */
int main(int argc, char *argv[]) 
{
    char s [] = "Hello, world!"; 
    char t [] = "Why hello there!";
    size_t len = string_length(s); 
    printf("Length of '%s' is %zu\n", s, len); 

    len = string_length(t); 
    printf("Length of '%s' is %zu\n", t, len); 

    int nOffset = atoi(argv[1]);

    char * pForce;
    pForce = (char *) t;

    printf("s: %p\n", s);
    printf("t: %p\n", t);
    printf("pForce: %p\n", pForce);

    strcpy(pForce+nOffset, "DARK SIDE!");
    len = string_length(s); 
    printf("Length of '%s' is %zu\n", s, len); 

    len = string_length(t); 
    printf("Length of '%s' is %zu\n", t, len); 

    return 0;
}



