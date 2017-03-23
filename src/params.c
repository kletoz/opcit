#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"

char **
params_split(char *text, int text_len, char *delim, int *params_num)
{
    char *s, *token, *copy, **params;
    int len, n;
    struct list *list = NULL, *node;

    for (s = text, n = 0; /* nothing */; s = NULL)
    {
        token = strtok(s, delim);

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

    params = params_split(input, input_length, " \t\n", params_num);

    free(input);

    return params;
}

void
params_destroy(char **params, int params_num)
{
    for (params_num--; params_num >= 0; params_num--)
        free(params[params_num]);
}


