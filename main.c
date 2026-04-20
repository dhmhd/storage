#include <stdio.h>

#include "storage.h"

typedef struct {
    int type;
    union {
        char *str_val;
        int int_val;
        double dbl_val;
    } value;
} val_t;

int print_el(void* element, size_t id, size_t i, size_t size, void* ctx)
{
    val_t *el = (val_t *)element;
    switch (el->type) {
        case 0:
            printf("'%s'", el->value.str_val);
            break;
        case 1:
            printf("%i", el->value.int_val);
            break;
        case 2:
            printf("%f", el->value.dbl_val);
            break;
    }
    int *limit = ctx;
    if (i < size-1 && *limit > 1)
    {
        printf("; ");
    }
    if (--(*limit) <= 0)
    {
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    storage_t storage;
    storage_init(&storage, 0, sizeof(val_t));

    size_t id;
    val_t *obj;
    obj = storage_push_back(&storage, &id);
    obj->type = 0;
    obj->value.str_val = "123";
    obj = storage_push_back(&storage, &id);
    obj->type = 1;
    obj->value.int_val = 123;
    obj = storage_push_back(&storage, &id);
    obj->type = 2;
    obj->value.dbl_val = 1.23;

    int limit;
    for (int i = 0; i < 100000000; ++i) {
        obj = storage_push_back(&storage, &id);
        obj->type = 2;
        obj->value.dbl_val = 1.23;
    }
    obj = storage_push_back(&storage, &id);
    obj->type = 0;
    obj->value.str_val = "123";
    limit = 10;
    storage_iter(&storage, print_el, &limit);
    printf("\n");
    for (int i = 0; i < 100000000; ++i) {
        storage_delete(&storage, i);
    }

    limit = 10;
    storage_iter(&storage, print_el, &limit);
    printf("\n");

    printf("%zu\n", storage.capacity);

    printf("Hi\n");
    return 0;
}