#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libcalc.h>

int
main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s version\n", argv[0]);
        exit(1);
    }

    if (strcmp(argv[1], "version") == 0) 
        version();
    else
        printf("Usage: %s version\n", argv[0]);
    
    return 0;
}
