struct list
{
    void *data;
    struct list *next;
};

struct list *list_prepend(struct list *, void *);
void list_destroy(struct list *);
