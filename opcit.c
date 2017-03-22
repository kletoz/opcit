#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libcalc.h>

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
    if (argc < 2)
        usage(argv[0]);

    if (strcmp(argv[1], "add") == 0)
    {
        if (argc == 4)
            op_add_cmd(argc, argv);
        else if (argc == 2)
            op_add_stdin();
        else
            usage(argv[0]);
    }
    else if (strcmp(argv[1], "sub") == 0 && argc == 4)
        op_sub(argc, argv);
    else if (strcmp(argv[1], "div") == 0 && argc == 4)
        op_div(argc, argv);
    else if (strcmp(argv[1], "mul") == 0 && argc == 4)
        op_mul(argc, argv);
    else if (strcmp(argv[1], "fibo") == 0 && argc == 3)
        op_fibo(argc, argv);
    else if (strcmp(argv[1], "addv") == 0 && argc == 4)
        op_addv(argc, argv);
    else if (strcmp(argv[1], "subv") == 0 && argc == 4)
        op_subv(argc, argv);
    else if (strcmp(argv[1], "mulv") == 0 && argc == 4)
        op_mulv(argc, argv);
    else
        usage(argv[0]);
    
    return 0;
}
