#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc.h"
#include "params.h"

void
usage(char *argv0)
{
    printf("Usage: %s version\n", argv0);
    printf("       %s <operator> <args>\n", argv0);
    printf("\n");
    printf("Arithmetic operators:\n");
    printf("    add, sub, div, mul\n");
    printf("\n");
    printf("Vector operators:\n");
    printf("    addv, subv, mulv\n");
    printf("\n");
    printf("Other operators:\n");
    printf("    fibo\n");
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

    if (strcmp(argv[1], "add") == 0 && params_num == 2)
        op_add(params[0], params[1]);
    else if (strcmp(argv[1], "sub") == 0 && params_num == 2)
        op_sub(params[0], params[1]);
    else if (strcmp(argv[1], "div") == 0 && params_num == 2)
        op_div(params[0], params[1]);
    else if (strcmp(argv[1], "mul") == 0 && params_num == 2)
        op_mul(params[0], params[1]);
    else if (strcmp(argv[1], "fibo") == 0 && params_num == 1)
        op_fibo(params[0]);
    else if (strcmp(argv[1], "addv") == 0 && params_num == 2)
        op_addv(params[0], params[1]);
    else if (strcmp(argv[1], "subv") == 0 && params_num == 2)
        op_subv(params[0], params[1]);
    else if (strcmp(argv[1], "mulv") == 0 && params_num == 2)
        op_mulv(params[0], params[1]);
    else if (strcmp(argv[1], "load") == 0 && params_num == 1)
        op_load(params[0]);
    else
        usage(argv[0]);
  
    if (!params_in_argv)
    {
        params_destroy(params, params_num);
        free(params);
    }

    return 0;
}
