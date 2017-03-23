#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc.h"
#include "util.h"

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

char **
params_split(char *text, int text_len, int *params_num)
{
    char *s, *token, *copy, **params;
    int len, n;
    struct list *list = NULL, *node;

    for (s = text, n = 0; /* nothing */; s = NULL)
    {
        token = strtok(s, " \t\n");

        if (token == NULL)
            break;

        len = strlen(token);

        if (token[len - 1] == '\n')
            token[--len] = '\0';

        if (len == 0)
            continue;

        copy = strdup(token);
        list = list_prepend(list, copy);
        n++;
    }

    params = malloc(n * sizeof(*params));
    *params_num = n;

    for (node = list; node; node = node->next)
        params[--n] = (char *) node->data;

    list_destroy(list);

    return params;
}

char **
params_read(int *params_num)
{
#define BUFSIZE 3

    char *s, *input, **params, buf[BUFSIZE];
    int input_size, input_length, slen, all_read = 0;

    input_length = 0;
    input_size = 10;
    input = malloc(input_size * sizeof(*input));

    do
    {
        s = fgets(buf, BUFSIZE, stdin);
        all_read = feof(stdin);

        if (s == NULL)
            break;

        slen = strlen(s);

        if (slen + input_length > input_size)
        {
            input_size += slen + 10;
            input = realloc(input, input_size * sizeof(*input));
        }

        strcpy(input + input_length, buf);

        input_length += slen;
    } while (!all_read);

    params = params_split(input, input_length, params_num);

    free(input);

    return params;
}

void
params_destroy(char **params, int params_num)
{
    for (params_num--; params_num >= 0; params_num--)
        free(params[params_num]);
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
    else
        usage(argv[0]);
  
    if (!params_in_argv)
    {
        params_destroy(params, params_num);
        free(params);
    }

    return 0;
}
