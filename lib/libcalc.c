#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
version(void)
{
    printf("calc v1.0\n");
}

void
op_add(int argc, char **argv)
{
    int x = atoi(argv[2]);
    int y = atoi(argv[3]);

    printf("%d\n", x + y);
}

void
op_sub(int argc, char **argv)
{
    int x = atoi(argv[2]);
    int y = atoi(argv[3]);

    printf("%d\n", x - y);
}

void
op_div(int argc, char **argv)
{
    int x = atoi(argv[2]);
    int y = atoi(argv[3]);

    printf("%d\n", x / y);
}

void
op_mul(int argc, char **argv)
{
    int x = atoi(argv[2]);
    int y = atoi(argv[3]);

    printf("%d\n", x * y);
}

void
fibonacci(int x)
{
    int i = 0, j = 1;

    printf("%d %d ", i, j);

    for (; x > 2; x--)
    {
        int t = i + j;
        i = j;
        j = t;
        printf("%d ", t);
    }
}

void
op_fibo(int argc, char **argv)
{
    int x = atoi(argv[2]);

    if (x > 0)
    {
        fibonacci(x);
        printf("\n");
    }
}

void
op_vector_add(int *x, int *y, int n)
{
    int i;

    for (i = 0; i <= n; i++)
        printf("%d ", x[i] + y[i]);
}

void
op_vector_sub(int *x, int *y, int n)
{
    int i;

    for (i = 0; i <= n; i++)
        printf("%d ", x[i] - y[i]);
}

void
op_vector_mul(int *x, int *y, int n)
{
    int i, a = 0;

    for (i = 0; i <= n; i++)
        a += x[i] * y[i];
        
    printf("%d ", a);
}

void
op_vector(int argc, char **argv, void (*op) (int *, int *, int))
{
    int i, n1, n2, len, *x, *y;
    char *str1, *str2, *token;

    str1 = argv[2];
    len = strlen(str1);

    for (i = n1 = 0; i < len; i++)
        if (str1[i] == ',')
            n1++;

    str2 = argv[3];
    len = strlen(str2);

    for (i = n2 = 0; i < len; i++)
        if (str2[i] == ',')
            n2++;

    if (n1 != n2)
    {
        printf("vector sizes don't match\n");
        exit(1);
    }
               
    x = malloc((n1 + 1) * sizeof(*x));
    y = malloc((n2 + 1) * sizeof(*y));

    for (i = 0, str1 = argv[2]; /* nothing */; i++, str1 = NULL)
    {
        token = strtok(str1, ",");
        
        if (token == NULL)
            break;
        
        x[i] = atoi(token);
    }

    for (i = 0, str2 = argv[3]; /* nothing */; i++, str2 = NULL)
    {
        token = strtok(str2, ",");
        
        if (token == NULL)
            break;
        
        y[i] = atoi(token);
    }

    op(x, y, n1);

    printf("\n"); 
}

void
op_addv(int argc, char **argv)
{
    op_vector(argc, argv, op_vector_add);
}

void
op_subv(int argc, char **argv)
{
    op_vector(argc, argv, op_vector_sub);
}

void
op_mulv(int argc, char **argv)
{
    op_vector(argc, argv, op_vector_mul);
}
