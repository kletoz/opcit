#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include "table.h"
#include "util.h"
#include "calc.h"
#include "params.h"

void
version(FILE *file)
{
    fprintf(file, "calc v1.0\n");
}

void
op_add(FILE *file, char *a, char *b)
{
    int x = atoi(a);
    int y = atoi(b);

    fprintf(file, "%d\n", x + y);
}

void
op_sub(FILE *file, char *a, char *b)
{
    int x = atoi(a);
    int y = atoi(b);

    fprintf(file, "%d\n", x - y);
}

void
op_div(FILE *file, char *a, char *b)
{
    int x = atoi(a);
    int y = atoi(b);

    printf("%d\n", x / y);
}

void
op_mul(FILE *file, char *a, char *b)
{
    int x = atoi(a);
    int y = atoi(b);

    fprintf(file, "%d\n", x * y);
}

void
fibonacci(FILE *file, int x)
{
    int i = 0, j = 1;

    fprintf(file, "%d %d ", i, j);

    flockfile(file);

    for (; x > 2; x--)
    {
        int t = i + j;
        i = j;
        j = t;
        fprintf(file, "%d ", t);
    }
        
    fprintf(file, "\n");
    funlockfile(file);
}

void
op_fibo(FILE *file, char *a)
{
    int x = atoi(a);

    if (x > 0)
        fibonacci(file, x);
}

void
op_vector_add(FILE *file, int *x, int *y, int n)
{
    int i;

    flockfile(file);
    
    for (i = 0; i <= n; i++)
        fprintf(file, "%d ", x[i] + y[i]);
    
    funlockfile(file);
}

void
op_vector_sub(FILE *file, int *x, int *y, int n)
{
    int i;

    flockfile(file);
    
    for (i = 0; i <= n; i++)
        fprintf(file, "%d ", x[i] - y[i]);
    
    funlockfile(file);
}

void
op_vector_mul(FILE *file, int *x, int *y, int n)
{
    int i, a = 0;

    for (i = 0; i <= n; i++)
        a += x[i] * y[i];
        
    fprintf(file, "%d ", a);
}

void
op_vector(FILE *file, char *a, char *b, void (*op) (FILE *, int *, int *, int))
{
    int i, n1, n2, len, *x, *y;
    char *str1, *str2, *token, *saveptr;

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
        token = strtok_r(str1, ",", &saveptr);
        
        if (token == NULL)
            break;
        
        x[i] = atoi(token);
    }

    for (i = 0, str2 = b; /* nothing */; i++, str2 = NULL)
    {
        token = strtok_r(str2, ",", &saveptr);
        
        if (token == NULL)
            break;
        
        y[i] = atoi(token);
    }

    op(file, x, y, n1);

    free(x);
    free(y);

    printf("\n"); 
}

void
op_addv(FILE *file, char *a, char *b)
{
    op_vector(file, a, b, op_vector_add);
}

void
op_subv(FILE *file, char *a, char *b)
{
    op_vector(file, a, b, op_vector_sub);
}

void
op_mulv(FILE *file, char *a, char *b)
{
    op_vector(file, a, b, op_vector_mul);
}

void
op_load(char *filename)
{
    table_load(filename);
}

void
op_search(FILE *file, char *token)
{
    table_search(file, token);
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

int
cmd_line_exec(char *line, int linelen, FILE *stream)
{
    int r, params_num;
    char **params;

    params = params_split(line, linelen, " \t\n", &params_num);
    r = cmd_exec(stream, params[0], params + 1, params_num - 1);
    params_destroy(params, params_num);

    return r;
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

    cmd_exec(stdout, params[0], params + 1, params_num - 1);
        
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

    free(threads);
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

/* Variável para controle do número de clientes conectados. Como cada cliente
 * pode alterar essa variável, além do loop principal, precisamos controlar o
 * seu acesso usando um mutex.
 *
 * Vamos usar também um variável de condição para sinalizar para a thread
 * principal (que cria os jobs) que cada thread chegou ao final. */ 
int server_clients_num;
pthread_mutex_t server_mutex;
pthread_cond_t server_condition;

/* Função executada por cada cliente. O servidor cria uma thread por cliente. */
void *
server_job(void *a)
{
    int buflen, connfd, bufsize;
    char *buf;
    FILE *stream;

    printf("server: client thread start\n");
    connfd = *((int *) a);

    /* Associa o file descriptor da conexão a uma stream do tipo FILE.
     *
     * Estamos usando isso apenas para reaproveitar todas as funções anteriores
     * que já existiam e estão sempre lidando com streams. Para sockets, no
     * entando, é mais comum usar as funções de nível mais baixo da bibliteca
     * padrão de I/O, que são read() e write(). */
    stream = fdopen(connfd, "a+");

    /* Lê uma linha do socket, como se fosse um arquivo. Estamos exigindo dessa
     * forma que o client envie uma quebra de linha no final da operação. */
    buf = NULL;

    while ((buflen = readline(&buf, &bufsize, stream)))
    {
        if (buflen == 0)
            continue;

        printf("client:[%s]\n", buf);
    
        /* Executa a linha enviada pelo cliente. */
        if (cmd_line_exec(buf, buflen, stream))
            fprintf(stream, "invalid command");

        fputc('\n', stream);
    }

    /* Libera o espeço alocado em readline() e verifica se há erro no stream. */
    free(buf);
   
    if (ferror(stream))
    {
        printf("socket(): %s\n", strerror(errno));
        exit(1);
    }

    fflush(stream);

    /* Fecha a conexão unilateralmente, derrubando a conexão. */
    fclose(stream);
    
    /* Libera a área de memória que foi passada para esta função, mas alocada na
     * função princial, que cria a thread op_server(). */
    free(a);

    printf("server: client thread finish\n");

    /* Diminui o número de clientes conectados no servidor, já que o cliente
     * dessa thread chegou ao fim. Depois, emite um sinal para a thread
     * principal (a única esperando).
     *
     * A thread principal quando recebe este sinal, pode verificar o número de
     * clientes ainda conectados. */
    pthread_mutex_lock(&server_mutex);
    server_clients_num--;
    pthread_cond_signal(&server_condition);
    pthread_mutex_unlock(&server_mutex);

    return 0;
}

/* Criação de um server simples que retorna o horário a cada nova conexão.
 *
 * A porta de conexão do server é passada como o parâmetro da função.
 *
 * Este é um tipo de implementação antigo, onde a estrutura de informação do
 * endereço (struct sockaddr_in) é preenchida manualmente.
 *
 * As implementações mais novas usam a função getaddrinfo(). Esta função foi
 * introduzida pela RFC 2553 em 1999. A função getaddrinfo() abstrai o
 * tratamento de IPv4 e IPv6.
 *
 * A estrutura de info nesta implementação (struct sockaddr_in) funciona para
 * IPv4. Para IPv6 deveríamos usar struct sockaddr_in6.
 *
 * Exite um texto muito bom explicando o funcionamento de sockets:
 *
 * Beej's Guide to Network Programming -- Using Internet Sockets
 * http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html 
 */
void
op_server(char *a)
{
    int *p, port, listenfd, connfd, optval, clients;
    struct sockaddr_in server, client;
    socklen_t clientlen;
    pthread_attr_t threadattr;
    pthread_t thread;

    /* Define um conjunto de atributos para thread do tipo DETACHED. Esse tipo
     * de thread não pode não será devolvido a quem chamou (essa função
     * op_server). Isto é, não é possível usar pthread_join() para thread
     * DETACHED. */
    pthread_attr_init(&threadattr);
    pthread_attr_setdetachstate(&threadattr, PTHREAD_CREATE_DETACHED);

    /* Definição (ou inicialização) da variável com o núemero de clientes
     * conectados e também das variáveis de controle mutex e cond. */ 
    server_clients_num = 0;
    pthread_mutex_init(&server_mutex, NULL);
    pthread_cond_init(&server_condition, NULL);

    /* Cria um socket com protocolo IPv4 (PF_INET) do tipo TCP (SOCK_STREAM). O
     * último parâmetro define o `protocolo'; zero significa que o protocolo
     * deve ser definido automaticamente a partir do tipo. Retorna um
     * file descriptor, assim como open(). */  
    listenfd = socket(PF_INET, SOCK_STREAM, 0);

    if (listenfd == -1)
    {
        printf("socket(): %s: %s\n", a, strerror(errno));
        exit(1);
    }

    optval = 1;

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   &optval, sizeof(optval)) == -1)
    {
        printf("setockopt(): SO_REUSEADDR: %s: %s\n", a, strerror(errno));
        exit(1);
    } 

    port = atoi(a);

    /* Inicia as variáveis de uma estrutura de informação de endereço de socket.
     * Essa estrutura será usada pela função bind() para ligar o file
     * descriptor do socket aberto ao endereço e porta configurado.
     *
     * O domínio do socket (family) é IPv4. Para o endereço (s_addr), estamos
     * usando INADDR_ANY que significa que o IP de localhost será usado no bind.
     *
     * A porta é definida com o que o usuário informou por linha de comando.
     *
     * Estamos usando as funções htonl() e htons() que converte a representação
     * de bytes do tipo da máquina (provavelmente little endian para o tipo de
     * representação de rede, definido como big endian. */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    /* Faz a ligação do file descriptor do socket ao endereço IP. */
    if (bind(listenfd, (struct sockaddr *) &server, sizeof(server)) == -1) 
    {
        printf("bind(): %s: %s\n", a, strerror(errno));
        exit(1);
    }

    /* Aguarda conexões no socket. Até 10 pedidos de conexão ficam em fila
     * aguardando. */
    if (listen(listenfd, 10) == -1)
    {
        printf("listen(): %s: %s\n", a, strerror(errno));
        exit(1);
    }

    clients = 1;

    /* Loop infinito de aceite de conexões. */
    while (1)
    {
        /* Aguarda um novo pedido de conexão. A execução fica parada nesse ponto
         * até que uma nova conexão seja feita na porta especificada.
         *
         * accept() retorna um novo file descriptor, que define o canal de
         * comunicação exclusivo com a nova conexão estabelecido.
         * 
         * O file descriptor do socket original continua ouvindo novas conexões.
         */
        clientlen = 0;
        connfd = accept(listenfd, (struct sockaddr *) &client, &clientlen);

        if (connfd == -1)
        {
            printf("accept(): %s: %s\n", a, strerror(errno));
            exit(1);
        }

        printf("server: connect from %s:%d\n", inet_ntoa(client.sin_addr),
               (unsigned short) ntohs(client.sin_port));

        p = malloc(sizeof(*p));
        *p = connfd;

        /* Controle do número de clientes conectados. */
        pthread_mutex_lock(&server_mutex);
        server_clients_num++;
        pthread_mutex_unlock(&server_mutex);

        /* Criação de uma thread com os atributos `threadattr' (que é DETACHED)
         * executando a função server_job e recebendo o ponteiro `p'. Este
         * ponteiro foi alocado aqui e deve ser liberado na função server_job.
         * Não liberar este ponteiro é uma forma de aumentar o consumo de
         * memória indefinidamente num server. */
        pthread_create(&thread, &threadattr, server_job, p); 
   
        /* Interrompe o servidor para um certo número de clientes. Usado apenas
         * para teste. Repare qua a variável usada `clients' não é a mesma
         * server_clients_num, para evitar mais um lock da variável. */
        if (clients++ > 2)
            break;
    }

    /* Fecha o socket de conexão. O fechamento deste socket não influencia os
     * clientes já conectados -- accept() retorna um fd para cada novo cliente.
     * Apenas novos clientes que tentarem se conectar serão impactados. */    
    close(listenfd);

    /* Aguarda o fim da execução das threads para cada cliente. Como as threads
     * são DETACHED, não podemo usar pthread_join(). Mas também não podemos
     * simplesmente sair desta função, pois isso retorna imeditamente para
     * main() e termina a execução da thread principal. Se isso acontecer, as
     * threads que ainda estiverem eventualmente execução serão interrompidas
     * imediatamente -- como um kill. */
    pthread_mutex_lock(&server_mutex);

    while (server_clients_num > 0)
        pthread_cond_wait(&server_condition, &server_mutex);
    
    pthread_mutex_unlock(&server_mutex);

    /* Libera a memória criada para as variáves de sincronização. */
    pthread_mutex_destroy(&server_mutex);
    pthread_cond_destroy(&server_condition);
    pthread_attr_destroy(&threadattr);
}

void
op_client(char *host, char *b, char *op)
{
    int port, sockfd, buflen, bufsize;
    struct sockaddr_in server;
    FILE *stream;
    char *buf;

    port = atoi(b);
    
    /* Cria o socket. */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
    {
        printf("socket(): %s:%d: %s\n", host, port, strerror(errno));
        exit(1);
    }

    /* Inicia a estrutura de informações com o domínio AF_INET (que é IPv4) e a
     * porta. Todos os outros elementos da estrutura devem ser 0 ou NULL (em
     * caso de ponteiros). Isso é feito de uma única vez com a função memset(),
     * que define como zero os N bytes da área de memória que começam em no byte
     * para onde *server aponta.
     *
     * `N' é o tamanho em bytes da estrutura struct sockaddr_in, calculado com
     * sizeof().
     *
     * Reparem que o memset é feito com 0 e não '0'. O primeiro, 0, é
     * representado como todos os bits zerados de um byte qualquer (seja ele
     * byte de inteiro, char, ponteiro). O segundo, '0', é uma representação
     * ascii e vale 48 (cf. man ascii). */
    memset(&server, 0, sizeof(server)); 
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
   
    /* Salva em sin_addr o IP passado por linha de comando (variável b) usando o
     * tipo IPv4 (AF_INET). Essa funçõa converte o IP de "printable" para
     * "network", ou pton. */ 
    if (inet_pton(AF_INET, b, &server.sin_addr) < 0)
    {
        printf("inet_pton(): %s: %s\n", host, strerror(errno));
        exit(1);
    }

    /* Conecta o socket sockfd ao servidor definido na estrutura `server'. */
    if (connect(sockfd, (struct sockaddr *) &server, sizeof(server)) == -1) 
    {
        printf("connect(): %s:%d: %s\n", host, port, strerror(errno));
        exit(1);
    }

    /* Converte o file descriptor da conexão para uma stream. Fazemos essa
     * conversão apenas para poder, na sequência, chamar as funções de stream,
     * como fgets() e printf(). Poderíamos, no entanto, usar diretamente as
     * funções equivalentes de file descriptors como read() e write(). A stream
     * associada tem o mode "ra+", que significa que pode ser lida e apendada.
     */
    stream = fdopen(sockfd, "a+");

    /* Escreve a operação lida na linha de comando para a stream, ou seja, envia
     * ao servidor. */
    fprintf(stream, "%s\n", op);
    fflush(stream);

    buf = NULL;

    buflen = readline(&buf, &bufsize, stream);
    
    if (buflen != 0)
        printf("%s", buf);

    free(buf);

    if (ferror(stream))
    {
        printf("%s\n", strerror(errno));
        exit(1);
    }

    /* Fecha a conexão. */
    fclose(stream);
}

int
cmd_exec(FILE *file, char *cmd, char **params, int params_num)
{
    int retval = 0;

    if (strcmp(cmd, "add") == 0 && params_num == 2)
        op_add(file, params[0], params[1]);
    else if (strcmp(cmd, "sub") == 0 && params_num == 2)
        op_sub(file, params[0], params[1]);
    else if (strcmp(cmd, "div") == 0 && params_num == 2)
        op_div(file, params[0], params[1]);
    else if (strcmp(cmd, "mul") == 0 && params_num == 2)
        op_mul(file, params[0], params[1]);
    else if (strcmp(cmd, "fibo") == 0 && params_num == 1)
        op_fibo(file, params[0]);
    else if (strcmp(cmd, "addv") == 0 && params_num == 2)
        op_addv(file, params[0], params[1]);
    else if (strcmp(cmd, "subv") == 0 && params_num == 2)
        op_subv(file, params[0], params[1]);
    else if (strcmp(cmd, "mulv") == 0 && params_num == 2)
        op_mulv(file, params[0], params[1]);
    else if (strcmp(cmd, "load") == 0 && params_num == 1)
        op_load(params[0]);
    else if (strcmp(cmd, "search") == 0 && params_num == 1)
        op_search(file, params[0]);
    else if (strcmp(cmd, "file") == 0 && params_num == 1)
        op_file(params[0]);
    else if (strcmp(cmd, "lines") == 0 && params_num == 1)
        op_lines(params[0]);
    else if (strcmp(cmd, "pillow") == 0 && params_num == 1)
        op_pillow(params[0]);
    else if (strcmp(cmd, "server") == 0 && params_num == 1)
        op_server(params[0]);
    else if (strcmp(cmd, "client") == 0 && params_num == 3)
        op_client(params[0], params[1], params[2]);
    else
        retval = 1;
    
    return retval;
}
