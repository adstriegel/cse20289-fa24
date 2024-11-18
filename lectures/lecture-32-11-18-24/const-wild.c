/* const-wild.c : Let's dance wildly - constant style */

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
    const char *s = "Hello, world!"; 
    const char *t = "Why hello there!";
    size_t len = string_length(s); 
    printf("Length of '%s' is %zu\n", s, len); 

    len = string_length(t); 
    printf("Length of '%s' is %zu\n", t, len); 

    char * pForce;
    pForce = (char *) s;

    strcpy(pForce+15, "Sprockets, now we dance");
    len = string_length(s); 
    printf("Length of '%s' is %zu\n", s, len); 

    len = string_length(t); 
    printf("Length of '%s' is %zu\n", t, len); 

    return 0;
}



