#include <stdlib.h>
#include <string.h>
#include "util.h"

struct list *
list_prepend(struct list *list, void *data)
{
    struct list *new;

    new = malloc(sizeof(*new));
    new->data = data;
    new->next = list;

    return new;
}

void
list_destroy(struct list *list)
{
    struct list *next = NULL;

    while (list != NULL)
    {
        next = list->next;
        free(list);
        list = next;
    }
}

/* Conta o número de linhas em um arquivo. */
int
lines_count(FILE *file)
{
#define BUFSIZE 10

    int len, lines;
    char *s, *end, buf[BUFSIZE];

    lines = 0;

    /* Lê no máximo BUFSIZE bytes para o buffer `buf' vindos do stream `file'. O
     * arquivo já deve ter sido aberto usando fopen() fora desta função. */
    while ((s = fgets(buf, BUFSIZE, file)))
    {
        /* Conta o tamanho da string lida em `buf'. A função fgets() garante que
         * após o último byte lido da stream será colocado um \0. Por isso
         * podemos usar a função de string strlen(). */
        len = strlen(buf);

        /* Ponteiro para o final da string do buffer (o '\0'). */
        end = s + len;

        /* Conta o número de caracteres de quebra de linha que existe no buffer
         * `buf'. Inicialmente `s' aponta para o início de `buf' (retornado
         * por fgets()). A cada iteração a função memchr() retorna um ponteiro
         * para o local onde o caractere '\n' foi encontrado.
         *
         * Se avançarmos esse ponteiro um byte iremos para a próxima posição em
         * `buf' e podemos repetir a chamada a memchr(), que ira procurar por um
         * '\n' no restante de `buf'.
         *
         * Como exemplo, suponha BUFSIZE igual a 10 e um arquivo com o seguinte
         * conteúdo:
         *
         * --8<----------
         * line 1
         * ln 2
         * lin 3
         * -->8----------
         *
         * Inicialmente o buf está vazio:
         *
         *     +---+---+---+---+---+---+---+---+---+---+
         * buf:| # | # | # | # | # | # | # | # | # | # |
         *     +---+---+---+---+---+---+---+---+---+---+
         *       0   1   2   3   4   5   6   7   8   9
         *
         * Após a primeira chamada a fgets(), o buf estará preenchido assim:
         *
         *       s                                   end
         *       |                                   |
         *       V                                   V
         *     +---+---+---+---+---+---+---+---+---+---+
         * buf:| l | i | n | e |   | 1 |\n | l | n |\0 | 
         *     +---+---+---+---+---+---+---+---+---+---+
         *       0   1   2   3   4   5   6   7   8   9
         *
         * com s apontando para o início de buf e end para o final da string.
         * Neste caso o final da string é o mesmo do buffer, mas não é
         * obrigatório que seja assim. Se houvesse, por exemplo, apenas a
         * primeira linha no arquivo ("line 1"), o '\0' estarai no índice 7 do
         * buffer. O fgets() que adiciona o '\0' após o '\n' ou no final do
         * buffer.
         *
         * A primeira chamada a função memchr() receberá s apontando para o
         * início de buf e retornará um novo ponteiro s' para o primeiro
         * caractere '\n'. O terceiro parâmetro da função memchr() é o número de
         * bytes que devem ser lidos. Estamos usando aritmética de ponteiros
         * para descobrir o tamanho do que deve ser passado. Na primeira
         * chamada, a subtração `end - s' tem exatamente o mesmo valor de `len'.
         *
         *       s                       s'          end
         *       |                       |           |
         *       V                       V           V
         *     +---+---+---+---+---+---+---+---+---+---+
         * buf:| l | i | n | e |   | 1 |\n | l | n |\0 | 
         *     +---+---+---+---+---+---+---+---+---+---+
         *       0   1   2   3   4   5   6   7   8   9
         *
         * O retorno de memchr() que estamos chamando de s' será atribuído ao
         * próprio s e teremos então, após a execução de memchr() o seguintes:
         *
         *                               s           end
         *                               |           |
         *                               V           V
         *     +---+---+---+---+---+---+---+---+---+---+
         * buf:| l | i | n | e |   | 1 |\n | l | n |\0 | 
         *     +---+---+---+---+---+---+---+---+---+---+
         *       0   1   2   3   4   5   6   7   8   9
         *
         * Incrementamos o contador lines e avançamos o ponteiro s um byte
         * (usando s++):
         *
         *                                   s       end
         *                                   |       |
         *                                   V       V
         *     +---+---+---+---+---+---+---+---+---+---+
         * buf:| l | i | n | e |   | 1 |\n | l | n |\0 | 
         *     +---+---+---+---+---+---+---+---+---+---+
         *       0   1   2   3   4   5   6   7   8   9
         *
         * Agora s aponta para a posição 7 de buf. A próxima chamada de
         * memchr() no while receberá, então, o restante do buf (posições 7, 8 e
         * 9). O tamanho do que falta ser lido é calculado pela aritimética de
         * ponteiros `end - s'.
         *
         * A segunda chamada a memchr() no loop não econtrará nenhum '\n' e
         * retornará NULL. Nessa segunda chamada o loop memchr() acaba. */
        while ((s = memchr(s, '\n', end - s)))
        {
            lines++;
            s++;
        }
    }
   
    if (ferror(file))
        return -1;

    return lines;
}
