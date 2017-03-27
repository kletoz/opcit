#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "table.h"
#include "util.h"
#include "calc.h"
#include "params.h"

int
cmd_exec(char *cmd, char **params, int params_num)
{
    int retval = 0;

    if (strcmp(cmd, "add") == 0 && params_num == 2)
        op_add(params[0], params[1]);
    else if (strcmp(cmd, "sub") == 0 && params_num == 2)
        op_sub(params[0], params[1]);
    else if (strcmp(cmd, "div") == 0 && params_num == 2)
        op_div(params[0], params[1]);
    else if (strcmp(cmd, "mul") == 0 && params_num == 2)
        op_mul(params[0], params[1]);
    else if (strcmp(cmd, "fibo") == 0 && params_num == 1)
        op_fibo(params[0]);
    else if (strcmp(cmd, "addv") == 0 && params_num == 2)
        op_addv(params[0], params[1]);
    else if (strcmp(cmd, "subv") == 0 && params_num == 2)
        op_subv(params[0], params[1]);
    else if (strcmp(cmd, "mulv") == 0 && params_num == 2)
        op_mulv(params[0], params[1]);
    else if (strcmp(cmd, "load") == 0 && params_num == 1)
        op_load(params[0]);
    else if (strcmp(cmd, "search") == 0 && params_num == 1)
        op_search(params[0]);
    else if (strcmp(cmd, "file") == 0 && params_num == 1)
        op_file(params[0]);
    else if (strcmp(cmd, "lines") == 0 && params_num == 1)
        op_lines(params[0]);
    else if (strcmp(cmd, "pillow") == 0 && params_num == 1)
        op_pillow(params[0]);
    else
        retval = 1;
    
    return retval;
}

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
    table_load(filename);
}

void
op_search(char *token)
{
    table_search(token);
}

int
readline(char **buf, int *bufsize, FILE *file)
{
    int slen, buflen;
    char *s;

    if (*buf == NULL)
    {
        *bufsize = 2;
        *buf = malloc(*bufsize * sizeof(*buf));
    }

    buflen = slen = 0;

    while ((s = fgets(*buf + buflen, *bufsize - buflen, file)))
    {
        if (s == NULL)
            break;

        slen = strlen(s);
        buflen += slen;

        if (s[slen - 1] == '\n')
        {
            break;
        }
        else
        {
            *bufsize += slen + 1;
            *buf = realloc(*buf, *bufsize * sizeof(*buf));
        }
    }

    return slen;
}

void *
job_cmd_exec(void *p)
{
    int len, params_num;
    char **params;
    char *buf = (char *) p;

    printf("> %s", buf);
        
    len = strlen(buf);
        
    params = params_split(buf, len, " \t\n", &params_num);

    cmd_exec(params[0], params + 1, params_num - 1);
        
    params_destroy(params, params_num);

    free(p);

    return 0;
}

void
op_file(char *filename)
{
    char *buf, *copy;
    int len, k, lines, threads_index, bufsize;
    FILE *file;
    pthread_t *threads;

    file = fopen(filename, "r");
    
    if (!file)
    {
        printf("%s: %s\n", filename, strerror(errno));
        exit(1);
    }

    lines = lines_count(file);

    if (lines < 0)
    {
        printf("%s: %s\n", filename, strerror(errno));
        exit(1);
    }

    /* A função lines_count() lê o arquivo completo e por isso o ponteiro do arquivo
     * depois da chamada está no final. Qualquer função que tentar ler esse
     * arquivo receberá EOF.
     *
     * A função fseek() da biblioteca padrão posiciona o ponteiro de leitura do
     * arquivo em algum ponto, definido pelos argumentos.
     *
     * Neste caso, estamos posicionando em 0 (segundo argumento) contando a
     * partir do início (SEEK_SET). Também é possível contar a partir do final
     * usando SEEK_END ou a partir da posição atual usandando SEEK_CUR. */
    fseek(file, 0, SEEK_SET);

    /* Para cada linha do arquivo será criada uma thread. Cada linha (ou
     * comando) é enviado a uma thread diferente. Por isso, precisamos de um
     * vetor de threads. */
    threads = malloc(lines * sizeof(*threads));
    threads_index = 0;

    buf = NULL;

    while ((len = readline(&buf, &bufsize, file)))
    {
        if (len == 0)
            continue;

        /* Cria um thread que vai executar a função job_cmd_exec. Nessa chamada,
         * job_cmd_exec é uma função definda com o protótipo
         *
         * void * (*job_cmd_exec) (void *)
         *
         * seguindo a especificação da biblioteca POSIX threads.
         *
         * O último parâmetro de pthread_create() é um ponteiro para void, ou
         * seja, um ponteiro para uma área de memória de qualquer tipo.
         *
         * Esse ponteiro será passado para a função job_cmd_exec(). Essa é a
         * forma de se passar argumento para a função executada na thread.
         *
         * Nessa implementação, vamos passar a linha completa lida para a thread
         * e o split em parâmetros será feito dentro de job_cmd_exec().
         *
         * Não podemos passar, porém, o ponteiro `buf' diretamente para a
         * thread. Este ponteiro aponta para uma área da memória criada pela
         * função readline(). Se a thread receber o mesmo ponteiro, o valor
         * dentro dessa área de memória será alterado pela nova chamada a
         * realine().
         *
         * Por isso, precismaos fazer uma cópia, para outra área de memória, do
         * conteúdo de `buf' antes de passar para job_cmd_exec().
         *
         * A função strdup() da biblioteca padrão cria uma nova área de memória
         * com uma cópia do conteúdo da string e retorna um ponteiro para essa
         * área de memória. */
        k = threads_index++;
        copy = strdup(buf);
        pthread_create(&threads[k], NULL, job_cmd_exec, copy);
    }

    free(buf);

    if (ferror(file))
    {
        printf("%s: %s\n", filename, strerror(errno));
        exit(1);
    }

    fclose(file);

    /* Aguardar todas as threads para continuar a execução. */
    for (k = 0; k < threads_index; k++)
        pthread_join(threads[k], NULL);
}

void
op_lines(char *filename)
{
    FILE *file;
    int lines;

    file = fopen(filename, "r");

    if (!file)
    {
        printf("%s: %s\n", filename, strerror(errno));
        exit(1);
    }

    lines = lines_count(file);

    if (lines < 0)
    {
        printf("%s: %s\n", filename, strerror(errno));
        exit(1);
    }

    fclose(file);

    printf("%d\n", lines);
}

/* Lista de personagens de Breaking Bed que fazem parte da cena Talking Pillow.
 *
 * Esse tipo de definição de um vetor de strings só é possível quando a
 * definição acontece ao mesmo tempo que a declaração. Se a declaração foi feita
 * primeiro e depois a definição, não compila.
 *
 * As chaves são usadas para atribuir valores para um vetor quando a definição é
 * feita ao mesmo tempo que a declaração. Funciona também para outros tipos:
 *
 * int a[3] = { 0, 1, 2 };
 *
 * O tamanho do vetor pode ser omitido nesses casos:
 *
 * int a[] = { 0, 1, 2 };
 *
 *
 * Exemplo de como NÃO FUNCIONA:
 *
 * char *characters[];
 * characters[0] = "adsada"; 
 */
char *characters[] = {
    "Walter White",
    "Skyler White",
    "Hank Schrader",
    "Marie Schrader",
    "Walter White, Jr."
};

/* Mutex para o controle de um conversa entre as threads. A thread que tiver o
 * lock deste mutex (ou tiver o travesseiro, se quiser pensar assim), pode
 * escrever no terminal o seu nome e um identificador. O identificador é dado
 * pelo processo-pai que cria as threads sequencialmente. */
pthread_mutex_t pillow;

/* Função executada por cada thread criada pelo processo-pai, através da função
 * op_pillow().
 *
 * Cada thread vai fazer uma impressão simples na tela, com um nome e um ID.
 *
 * O nome é obtido de uma lista de nomes (um vetor de ponteiros para strings, ou
 * ainda um ponteiro para ponteiros de char) definada estaticamente logo acima,
 * em characters.
 */
void *
pillow_talking(void *s)
{
    /* Esta pode ser uma operação complicada de se ler em C. Vamos quebra-la em
     * partes.
     *
     * Primeiro a conversão por casting. O argumento desta função é um void *. O
     * que significa que é um ponteiro apontando para uma área da memória de
     * tipo desconhecido. Ou seja, é um ponteiro para bytes, que não informamos
     * ao compilador de que tipo, se inteiros, ponto flutuando ou characteres.
     *
     * Mas como nós que fizemos a chamada a esta função através de
     * pthread_create(), nós sabemos que o último argumento que passamos foi um
     * inteiro. Lembrando que o último argumento passado para pthread_create() é
     * um ponteiro que será o argumento desta função aqui, a "função da thread".
     *
     * Então, podemos converter o ponteiro da área de memória de tipo
     * desconhecido para um ponteiro para uma área de memória de inteiro:
     *
     *     (int *) s
     *
     * Essa conversão por casting retorna um ponteiro para inteiro. Poderiamos
     * dar um nome para esse ponteiro, inclusive:
     *
     *     int *p = (int *) s;
     *
     * Dessa forma, conseguimos fazer operações no ponteiro p.
     *
     * A segunda parte da expressão é usada para obter o valor da área de
     * memória para onde o ponteiro s (ou neste texto o novo p) aponta. 
     *
     * A operação unária '*' antes de um ponteiro retorna o valor da área de
     * memória para onde o ponteiro aponta.
     *
     * Usamos, então, esse operador para obter o valor do ponteiro '(int *) s'.
     * Os parêntes são usados para garantir a precedência que queremos. O
     * ponteiro que queremos é o ((int *) s). Mas como queremos o valor para
     * onde aponta, usando o operador '*' no início, chegando em
     *
     *    *((int *) s)
     */
    int k = *((int *) s);
    int i = k % 5;

    pthread_mutex_lock(&pillow);

    printf("(%d) %s: Alright, I've got the talking pillow now... Okay?\n", k, characters[i]);

    pthread_mutex_unlock(&pillow);

    free(s);

    return NULL;
}

void
op_pillow(char *a)
{
    int i, *k, n;
    pthread_t *threads;

    n = atoi(a);

    pthread_mutex_init(&pillow, NULL);
    threads = malloc(n * sizeof(*threads));

    for (i = 0; i < n; i++)
    {
        /* Precisamos criar uma área de memória diferente para cada thread. Como
         * passamos um ponteiro para uma área de memória em pthread_create(),
         * não podemos passar o endereço da variável i, pois essa variável será
         * alterada a cada iteração do loop.
         *
         * Criamos então uma nova área de memória e passamos o valor de i para a
         * área de memória. O ponteiro para esta nova área de memória, por sua
         * vez, é passado para a função pthread_create(). */
        k = malloc(sizeof(int));
        *k = i;
        pthread_create(&threads[i], NULL, pillow_talking, k);
    }

    for (i = 0; i < n; i++)
        pthread_join(threads[i], NULL);

    pthread_mutex_destroy(&pillow);

    free(threads);
}
