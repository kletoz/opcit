#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libcalc.h>

void
usage(char *argv0)
{
    printf("Usage: %s version\n", argv0);
    exit(1);
}

int
main(int argc, char *argv[])
{
    if (argc != 2)
        usage(argv[0]);

    if (strcmp(argv[1], "version") == 0) 
        version();
    else
        usage(argv[0]);
    
    return 0;
}
