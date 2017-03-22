#include <stdlib.h>
#include <libutil.h>

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
