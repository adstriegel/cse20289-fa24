/* const-dance.c : Let's dance - constant style */

#include <stdio.h>
#include <stdlib.h>

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
    size_t len = string_length(s); 
    printf("Length of '%s' is %zu\n", s, len); 
    return 0;
}



