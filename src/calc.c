#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
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

#define SHM_FILE_KEY ".shmkey"

/* Início de um segmento de shared memory System V.
 *
 * Existem duas bibliotecas diferentes para shared memory: System V e POSIX.
 *
 * Em POSIX as funções são do tipo shm_open(). -- man shm_overview
 *
 * System V é mais antiga e implementada em todos os sistemas, diferente de
 * POSIX. Apesar de POSIX ser "melhor" o uso de SysV é mais comum. */ 
void *
shm_init(size_t size, int flag)
{
    key_t key;
    FILE *filekey;
    void *shm;
    int id;

    /* Cria um arquivo se não existir para ser usado como chave da shared
     * memory. */
    filekey = fopen(SHM_FILE_KEY, "a");
    fclose(filekey);

    /* Obtém um inteiro provavelmente único a partir do nome do arquivo. Apenas
     * uma parte do inode do arquivo é usada de fato, o que pode produzir
     * conflito, apesar de improvável. */
    key = ftok(SHM_FILE_KEY, 'x');

    if (key == -1)
    {
        printf("ftok(): %s: %s\n", SHM_FILE_KEY, strerror(errno));
        exit(1);
    }

    /* Obtém um segmento de shared memory com a chave especificada. Se o
     * segmento não existir será criado desde que 'flag' contenha IPC_CREAT.
     * O tamanho da shared memory é sempre definido na criação, nesse caso com
     * 'size' bytes. */
    id = shmget(key, size, flag | 0660);

    if (id == -1)
    {
        printf("shmget(): %s: %zd bytes: %s\n", SHM_FILE_KEY, size,
               strerror(errno));
        exit(1);
    }

    /* Retorna o ponteiro do segmento de shared memory, ou vendo de outra forma,
     * shmat() anexa o segmento de shared memory ao endereço de memória local do
     * processo. */
    shm = shmat(id, NULL, 0);

    if (shm == (void *) -1)
    {
        printf("shmat(): %s: %s\n", SHM_FILE_KEY, strerror(errno));
        exit(1);
    }

    return shm;
}

#define SEM_NUM 2

enum sem
{
    SEM_READER,
    SEM_WRITER
};

/* Início de um conjunto de semáforos.
 *
 * Os semafóros são usados para o controle de acesso a shared memory.
 *
 * Temos dois tipos processos que podem acessar a shared memory:
 *
 * opcit load <arquivo>
 * opcit search <string>
 *
 * O processo load é de escrita e o processo search é de leitura exclusivamente.
 * Apesar de termos dois tipos de processos, podemos ter vários processos
 * executando ao mesmo tempo usando a shared memory. Por exemplo, podemos
 * executar duas ou mais cargas simultaneamente e também várias buscas ao mesmo
 * tempo.
 *
 * Vamos seguir a regra básica de semáforos:
 *
 * 1) o acesso de escrita deve ser exclusivo (apenas um escritor por vez)
 * 2) o acesso de escrita bloqueia todas as leituras
 * 3) as leituras podem acontecer todas ao mesmo tempo, sem bloqueio
 * 4) a prioridade é de escrita
 *
 * São criados dois semáforos: uma para controlar a escrita e outro a leitura.
 * Os dois semáforos estão no mesmo segmento. */
int
sem_init(int role)
{
    int i, id, flag, already_exists;
    key_t key;
    FILE *filekey;
    unsigned short semun_array[SEM_NUM];

    /* Cria um arquivo se não existir para ser usado como chave dos semáforos.
     * Convenientemente estamos usando o mesmo arquivo do segmento de shared
     * memory. */
    filekey = fopen(SHM_FILE_KEY, "a");
    fclose(filekey);

    key = ftok(SHM_FILE_KEY, 'x');

    if (key == -1)
    {
        printf("ftok(): %s: %s\n", SHM_FILE_KEY, strerror(errno));
        exit(1);
    }

    /* Se o processo for de escrita, usar as flags de criação exclusiva. Ou
     * seja, o conjunto de semáforos só será criado se não existir. */ 
    if (role == SEM_WRITER)
        flag = IPC_CREAT | IPC_EXCL | 0660;
    else
        flag = 0660;

    already_exists = 0;
    id = semget(key, SEM_NUM, flag);

    /* Se o processo for de escrita e semget() não conseguiu criar os semáforos
     * exclusivamente (ou seja, eles já existem), obtém o id dos semáforos já
     * existentes. */
    if (id == -1 && role == SEM_WRITER && errno == EEXIST)
    {
        already_exists = 1;
        flag = 0660;
        id = semget(key, SEM_NUM, flag);
    }
    
    if (id == -1)
    {
        printf("semget(): %s: %d: %s\n", SHM_FILE_KEY, key,
               strerror(errno));
        exit(1);
    }

    /* Caso o processo seja de escrita e a criação dos semáforos aconteceu nesta
     * execução, isto é, os semáforos ainda não existiam e foram criados agora,
     * definir todos os semáforos do conjunto (2 neste caso) como 0. Zero
     * significa que o processo pode continuar. Dessa forma, o primeiro processo
     * que acessar o semáforo poderá continuar.
     *
     * Os acessos seguintes que sejam de escrita (role SEM_WRITER) não podem
     * alterar o valor dos semáforos para zero novamente, ou todo o controle
     * seria perdido. */
    if (role == SEM_WRITER && !already_exists)
    { 
        for (i = 0; i < SEM_NUM; i++)
            semun_array[i] = 0;

        if (semctl(id, 1, SETALL, semun_array) == -1)
        {
            printf("semctl(): %s: %d: %s\n", SHM_FILE_KEY, key,
                   strerror(errno));
            exit(1);
        }
    }

    return id;
}

void
table_write_lock(int semid)
{
    struct sembuf ops[3];

    /* Para obter o lock de escrita, são feitas três operações nos 2 semáforos.
     * 
     * A primeira operação é sem_op 0, que significa aguardar até que o semáforo
     * de escrita esteja livre. Estamos definindo o semáforo de escrita como o
     * que tem sem_num igual a SEM_WRITER -- ou 1, de acordo com nossa enum sem.
     *
     * Quando o semáforo SEM_WRITER estiver liberado (atingir zero), nos fazemos
     * uma outra operação, que é a sem_op 1 no semáforo SEM_WRITER. Esta
     * operação no semáforo trava o semáforo para outro processo que chegar
     * depois desse.
     *
     * Além de travar o semáforo de escrita SEM_WRITER, fazemos mais uma
     * operação, que é aguardar o semáforo de leitura SEM_READER (ou 0 na enum
     * sem) ser liberado. Ou seja, antes de escrever vamos aguardar todos os
     * processos que estiverem escrevendo terminar e *também* aguardar todos os
     * processos que estiverem lendo terminar. 
     *
     * Quando todos os processos de leitura tiverem terminado (operação 3,
     * ops[2]), além dos de escrita (operação 1, ops[0]), podemos seguir com a
     * ação de escrita na memória. */
    ops[0].sem_num = SEM_WRITER;
    ops[0].sem_op = 0;
    ops[0].sem_flg = SEM_UNDO;
    
    ops[1].sem_num = SEM_WRITER;
    ops[1].sem_op = 1;
    ops[1].sem_flg = SEM_UNDO;
    
    ops[2].sem_num = SEM_READER;
    ops[2].sem_op = 0;
    ops[2].sem_flg = SEM_UNDO;

    if (semop(semid, ops, 3) == -1)
    {
        printf("semopget(): %s: write lock: %s\n", SHM_FILE_KEY,
               strerror(errno));
        exit(1);
    }
}

void
table_write_unlock(int semid)
{
    struct sembuf ops[1];
   
    /* A liberação de um lock de escrita é mais simples. Apenas uma operação é
     * necessária em um único semáforo.
     *
     * A operação é liberar o semáforo, colocando sem_op -1.
     *
     * Estamos usando -1 aqui porque usamos sem_op 1 no lock. Mas pode ser usado
     * qualquer inteiro, desde que seja o mesmo em módulo. */
    ops[0].sem_num = SEM_WRITER;
    ops[0].sem_op = -1;
    ops[0].sem_flg = SEM_UNDO;
    
    if (semop(semid, ops, 1) == -1)
    {
        printf("semopget(): %s: write unlock: %s\n", SHM_FILE_KEY,
               strerror(errno));
        exit(1);
    }
}

void
table_read_lock(int semid)
{
    struct sembuf ops[2];

    /* Para obter o lock de leitura fazemos duas operações em dois semáforos.
     *
     * Primeira operação é aguardar o semáforo de escrita ser liberado. Está é a
     * sem_op 0.
     *
     * Quando o semáforo de escrita for zero, podemos incrementar o semáforo de
     * leitura com sem_op 1.
     *
     * Não precisamos aguardar a fila de leitores pois o acesso de leitura pode
     * ser concorrente. O semáforo SEM_READER e incrementado para avisar para o
     * próximo escritor que estamos ocupando o recurso controlado pelo semáforo
     * (shared memory, no caso). */
    ops[0].sem_num = SEM_WRITER;
    ops[0].sem_op = 0;
    ops[0].sem_flg = SEM_UNDO;
    
    ops[1].sem_num = SEM_READER;
    ops[1].sem_op = 1;
    ops[1].sem_flg = SEM_UNDO;
    
    if (semop(semid, ops, 2) == -1)
    {
        printf("semopget(): %s: read lock: %s\n", SHM_FILE_KEY,
               strerror(errno));
        exit(1);
    }
}

void
table_read_unlock(int semid)
{
    struct sembuf ops[1];
   
    /* A liberação de um lock de leitura é apenas decrementar o valor do
     * semáforo de leitura. */ 
    ops[0].sem_num = SEM_READER;
    ops[0].sem_op = -1;
    ops[0].sem_flg = SEM_UNDO;
    
    if (semop(semid, ops, 1) == -1)
    {
        printf("semopget(): %s: read unlock: %s\n", SHM_FILE_KEY,
               strerror(errno));
        exit(1);
    }
}

#define CONTACTS_NUM 10
#define CONTACTS_NAME_SIZE 50
#define CONTACTS_EMAIL_SIZE 50

struct contacts
{
    char name[CONTACTS_NAME_SIZE];
    char email[CONTACTS_EMAIL_SIZE];
};

void
op_load(char *filename)
{
#define BUFSIZE 10

    char *s, *input, *line, **params, buf[BUFSIZE];
    int k, *kp, slen, lineno, input_length, input_size, params_num, sem;
    void *shm;
    struct contacts *contacts;
    FILE *file = fopen(filename, "r");

    if (!file)
    {
        printf("%s: %s\n", filename, strerror(errno));
        exit(1);
    }

    shm = shm_init(CONTACTS_NUM * sizeof(*contacts), IPC_CREAT);
    sem = sem_init(SEM_WRITER);
    
    kp = (int *) shm;
    contacts = (struct contacts *) (shm + sizeof(*kp));
    k = 0;

    input_length = 0;
    input_size = 10;
    input = malloc(input_size * sizeof(*input));
    lineno = 0;
    
    table_write_lock(sem);

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
                if (k > CONTACTS_NUM)
                {
                    printf("%s: too many contacts, loaded only first %d\n", filename,
                           CONTACTS_NUM);
                    break;
                }
                else
                {
                    /* Copiar somente o número de bytes que estão disponíveis
                     * para o campo na shared memory (struct contacts).
                     * 
                     * A função strncpy() não coloca o terminador \0 no final da
                     * string de destino. strncpy() apenas copia no máximo N
                     * bytes para o destino. Se nos primeiros N bytes da origem
                     * existir um \0, será copiado normalmente.
                     *
                     * Por isso, é preciso adicionar um \0 separadamente na
                     * string de destino. Assim, garantimos que a string de
                     * destino é válida (termina com \0). */
                    strncpy(contacts[k].name, params[0], CONTACTS_NAME_SIZE);
                    contacts[k].name[CONTACTS_NAME_SIZE - 1] = '\0';
                    strncpy(contacts[k].email, params[1], CONTACTS_EMAIL_SIZE);
                    contacts[k].name[CONTACTS_EMAIL_SIZE - 1] = '\0';
                    k++;
                }
            }

            params_destroy(params, params_num);
        }
    }

    table_write_unlock(sem);

    *kp = k;

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
    int i, k, sem;
    void *shm;
    struct contacts *contacts;
    
    shm = shm_init(CONTACTS_NUM * sizeof(*contacts), 0);
    sem = sem_init(SEM_READER);
  
    table_read_lock(sem);

    /* Lê o início do segmento de shared memory como um inteiro.
     * O ponteiro para o segmento de memório é do tipo void *, ou seja, não tem
     * um tipo específico, é só memória.
     *
     * O casting ((int *) shm) converte esse ponteiro para um inteiro, dizendo,
     * então, que o ponteiro para a shared memory é um ponteiro para inteiro.
     * 
     * Como temos, depois do casting, um ponteiro para inteiro, podemos ler o
     * valor do inteiro armazenado na área de memória que esse ponteiro aponta
     * fazendo o desreferenciamento. O operador de desreferenciamento é o '*'.
     *
     * Outra forma mais explícita de escrever isso seria:
     *
     * int *p, k;
     *
     * p = (int *) shm;
     * k = *p;
     */
    k = *((int *) shm);
    contacts = (struct contacts *) (shm + sizeof(k));

    /* Imprimir todos os contantos que estão na shared memory. Como não existe
     * nenhuma forma de saber quantos contatos estão armazenados, fazemos um
     * loop por todos os 100 registros possíveis (que é o espaço aloca na shared
     * memory). Imprimimos cada registro que não for vazio. */
    if (strcmp(token, "all") == 0)
        for (i = 0; i < k; i++)
            printf("%-50s | %-50s\n", contacts[i].name, contacts[i].email);
    
    table_read_unlock(sem);
}
