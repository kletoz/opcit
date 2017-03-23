#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void
version(void)
{
    printf("calc v1.0\n");
}

void
op_add(char *a, char *b)
{
    int x = atoi(a);
    int y = atoi(b);

    printf("%d\n", x + y);
}

void
op_sub(char *a, char *b)
{
    int x = atoi(a);
    int y = atoi(b);

    printf("%d\n", x - y);
}

void
op_div(char *a, char *b)
{
    int x = atoi(a);
    int y = atoi(b);

    printf("%d\n", x / y);
}

void
op_mul(char *a, char *b)
{
    int x = atoi(a);
    int y = atoi(b);

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
op_fibo(char *a)
{
    int x = atoi(a);

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
op_vector(char *a, char *b, void (*op) (int *, int *, int))
{
    int i, n1, n2, len, *x, *y;
    char *str1, *str2, *token;

    str1 = a;
    len = strlen(str1);

    for (i = n1 = 0; i < len; i++)
        if (str1[i] == ',')
            n1++;

    str2 = b;
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

    for (i = 0, str1 = a; /* nothing */; i++, str1 = NULL)
    {
        token = strtok(str1, ",");
        
        if (token == NULL)
            break;
        
        x[i] = atoi(token);
    }

    for (i = 0, str2 = b; /* nothing */; i++, str2 = NULL)
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
op_addv(char *a, char *b)
{
    op_vector(a, b, op_vector_add);
}

void
op_subv(char *a, char *b)
{
    op_vector(a, b, op_vector_sub);
}

void
op_mulv(char *a, char *b)
{
    op_vector(a, b, op_vector_mul);
}

void
op_load(char *filename)
{
#define BUFSIZE 10

    char *s, *input, *line, buf[BUFSIZE];
    int slen, lineno, input_length, input_size;
    FILE *file = fopen(filename, "r");

    if (!file)
    {
        printf("%s: %s\n", filename, strerror(errno));
        exit(1);
    }
   
    input_length = 0;
    input_size = 10;
    input = malloc(input_size * sizeof(*input));
    lineno = 0;

    while ((s = fgets(buf, BUFSIZE, file)))
    {
        slen = strlen(buf);
        line = NULL;

        /* Se o input estiver vazio e o último caracter lido para o buffer for
         * '\n', então a linha inteira já está no buffer.
         *
         * Caso contrário, só um pedaço da linha foi lido (devido a limitação de
         * tamanho do buffer -- BUFSIZE). Neste caso, salvar o pedaço que já foi
         * lido num vetor de alocação dinânima e continuar lendo.
         *
         * Quando o vetor de alocação dinâmica já estiver com alguma parte
         * preenchida (input_length diferente de zero), concatenar o que já
         * havia sido lido com o que acabou de ser lido. Se o tamanho da linha
         * for maior do que o espaço já reservado (input_size), realocar a
         * memória.
         * */
        if (input_length == 0 && buf[slen - 1] == '\n')
        {
            line = buf;
        }
        else
        {
            if (slen + input_length > input_size)
            {
                input_size += slen + 10;
                input = realloc(input, input_size * sizeof(*input));
            }

            strcpy(input + input_length, buf);
            input_length += slen;

            if (buf[slen - 1] == '\n')
            {
                line = input;
                slen = input_length;
                input_length = 0;
            }
        }

        if (line)
        {
            lineno++;
            
            if (line[slen - 1] == '\n')
                line[--slen] = '\0';
          
            printf("%s:%d: [%s]\n", filename, lineno, line);
        }
    }

    if (ferror(file))
    {
        printf("%s: %s\n", filename, strerror(errno));
        exit(1);
    }

    fclose(file); 
}
