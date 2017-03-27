#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc.h"
#include "params.h"

void
usage(char *argv0)
{
    printf("Usage: %s <operator> <args>\n", argv0);
    printf("\n");
    printf("README file has more details.\n");
    exit(1);
}

int
main(int argc, char *argv[])
{
    int params_num, params_in_argv;
    char **params;

    if (argc < 2)
        usage(argv[0]);

    if (argc == 2)
    {
        params = params_read(&params_num);
        params_in_argv = 0;
    }
    else
    {
        params_num = argc - 2;
        params = argv + 2;
        params_in_argv = 1;
    }

    if (cmd_exec(argv[1], params, params_num))
        usage(argv[1]);
  
    if (!params_in_argv)
    {
        params_destroy(params, params_num);
        free(params);
    }

    return 0;
}
