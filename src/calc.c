#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "params.h"

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

void *
shm_init(size_t size, int flag)
{
#define SHM_FILE_KEY ".shmkey"

    key_t key;
    FILE *filekey;
    void *shm;
    int id;

    filekey = fopen(SHM_FILE_KEY, "a");
    fclose(filekey);

    key = ftok(SHM_FILE_KEY, 'x');

    if (key == -1)
    {
        printf("ftok(): %s: %s\n", SHM_FILE_KEY, strerror(errno));
        exit(1);
    }

    id = shmget(key, size, flag | 0660);

    if (id == -1)
    {
        printf("shmget(): %s: %zd bytes: %s\n", SHM_FILE_KEY, size,
               strerror(errno));
        exit(1);
    }

    shm = shmat(id, NULL, 0);

    if (shm == (void *) -1)
    {
        printf("shmat(): %s: %s\n", SHM_FILE_KEY, strerror(errno));
        exit(1);
    }

    return shm;
}

struct contacts
{
    char name[50];
    char email[50];
};

void
op_load(char *filename)
{
#define BUFSIZE 10

    char *s, *input, *line, **params, buf[BUFSIZE];
    int k, slen, lineno, input_length, input_size, params_num;
    void *shm;
    struct contacts *contacts;
    FILE *file = fopen(filename, "r");

    if (!file)
    {
        printf("%s: %s\n", filename, strerror(errno));
        exit(1);
    }

    shm = shm_init(100 * (50 + 50), IPC_CREAT);
    contacts = (struct contacts *) shm;
    k = 0;

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
        
            params = params_split(line, slen, ",", &params_num);

            if (params_num != 2)
            {
                printf("%s:%d: warning: invalid contact\n", filename, lineno);
            }
            else
            {
                strcpy(contacts[k].name, params[0]); 
                strcpy(contacts[k].email, params[1]);
                k++; 
            }

            printf("%s | %s\n", params[0], params[1]);
            params_destroy(params, params_num);
        }
    }

    if (ferror(file))
    {
        printf("%s: %s\n", filename, strerror(errno));
        exit(1);
    }

    fclose(file);
    free(input); 
}

void
op_search(char *token)
{
    int i;
    void *shm;
    struct contacts *contacts;
    
    shm = shm_init(100 * (50 + 50), 0);
    contacts = (struct contacts *) shm;

    /* Imprimir todos os contantos que estão na shared memory. Como não existe
     * nenhuma forma de saber quantos contatos estão armazenados, fazemos um
     * loop por todos os 100 registros possíveis (que é o espaço aloca na shared
     * memory). Imprimimos cada registro que não for vazio. */
    if (strcmp(token, "all") == 0)
        for (i = 0; i < 100; i++)
            if (strlen(contacts[i].name))
                printf("%-50s | %-50s\n", contacts[i].name, contacts[i].email);
}
