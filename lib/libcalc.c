#include <stdio.h>
#include <stdlib.h>

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
