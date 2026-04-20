#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct storage
{
    uint8_t* data;
    size_t* rindex;
    size_t* index;
    size_t element_size;
    size_t capacity;
    size_t size;
    size_t id_capacity;
    size_t cur_id;
    uint8_t* swap_place;
} storage_t;

static inline int storage_init(
    storage_t* storage,
    size_t initial_capacity,
    size_t element_size)
{
    memset(storage, 0, sizeof(*storage));
    if (element_size == 0)
    {
        return 0;
    }
    size_t capacity = initial_capacity;
    if (capacity == 0)
    {
        capacity = 1;
    }
    if (capacity > SIZE_MAX / element_size)
    {
        return 0;
    }
    storage->element_size = element_size;
    storage->capacity = capacity;
    storage->data = (uint8_t*)malloc(capacity * element_size);
    if (!storage->data)
    {
        memset(storage, 0, sizeof(*storage));
        return 0;
    }
    storage->swap_place = (uint8_t*)malloc(element_size);
    if (!storage->swap_place)
    {
        free(storage->data);
        memset(storage, 0, sizeof(*storage));
        return 0;
    }
    return 1;
}

static inline void storage_free(storage_t* storage)
{
    free(storage->data);
    free(storage->rindex);
    free(storage->index);
    free(storage->swap_place);
    memset(storage, 0, sizeof(*storage));
}

static inline int storage_grow(storage_t* storage)
{
    if (storage->size < storage->capacity)
    {
        return 1;
    }
    size_t new_cap = storage->capacity * 2;
    size_t new_len = new_cap * storage->element_size;
    uint8_t* new_data = (uint8_t*)realloc(storage->data, new_len);
    if (!new_data)
    {
        return 0;
    }
    storage->data = new_data;
    storage->capacity = new_cap;
    return 1;
}

static inline int storage_get_new_id(storage_t* storage, size_t* ret_id)
{
    if (storage->cur_id > storage->size)
    {
        *ret_id = storage->rindex[storage->size];
        return 1;
    }
    size_t new_id = storage->cur_id;
    if (new_id == SIZE_MAX)
    {
        return 0;
    }
    if (new_id == storage->id_capacity)
    {
        size_t new_cap = storage->id_capacity ? storage->id_capacity * 2 : 4;
        if (new_cap <= storage->id_capacity)   /* *2 overflowed */
        {
            new_cap = new_id + 1;
        }
        if (new_cap > SIZE_MAX / sizeof(size_t))
        {
            return 0;
        }
        size_t new_len = new_cap * sizeof(size_t);
        size_t* tmp = (size_t*)realloc(storage->index, new_len);
        if (!tmp)
        {
            return 0;
        }
        storage->index = tmp;
        tmp = (size_t*)realloc(storage->rindex, new_len);
        if (!tmp)
        {
            return 0;
        }
        storage->rindex = tmp;
        storage->id_capacity = new_cap;
    }
    storage->rindex[new_id] = new_id;
    storage->index[new_id] = storage->size;
    storage->cur_id = new_id + 1;
    *ret_id = new_id;
    return 1;
}

static inline void* storage_push_back(storage_t* storage, size_t *ret_id)
{
    if (!storage_grow(storage))
    {
        return NULL;
    }
    if (!storage_get_new_id(storage, ret_id))
    {
        return NULL;
    }
    size_t data_idx = storage->size;
    ++storage->size;
    storage->index[*ret_id] = data_idx;
    storage->rindex[data_idx] = *ret_id;
    void* data = storage->data + data_idx * storage->element_size;
    return data;
}

static inline void storage_delete(storage_t* storage, size_t id)
{
    if (id >= storage->cur_id)
    {
        return;
    }
    size_t data_idx = storage->index[id];
    if (data_idx >= storage->size)
    {
        return;
    }
    size_t last_data_idx = storage->size - 1;
    size_t last_id = storage->rindex[last_data_idx];
    if (data_idx != last_data_idx)
    {
        size_t sz = storage->element_size;
        memcpy(storage->swap_place, storage->data + data_idx * sz, sz);
        memcpy(storage->data + data_idx * sz, storage->data + last_data_idx * sz, sz);
        memcpy(storage->data + last_data_idx * sz, storage->swap_place, sz);
    }
    size_t tmp_rindex = storage->rindex[data_idx];
    storage->rindex[data_idx] = storage->rindex[last_data_idx];
    storage->rindex[last_data_idx] = tmp_rindex;
    storage->index[id] = last_data_idx;
    storage->index[last_id] = data_idx;
    storage->size--;
}

static inline void* storage_get(storage_t *storage, size_t id)
{
    if (id >= storage->cur_id)
    {
        return NULL;
    }
    size_t data_idx = storage->index[id];
    if (data_idx >= storage->size)
    {
        return NULL;
    }
    return storage->data + data_idx * storage->element_size;
}

static inline void storage_clear(storage_t* storage)
{
    storage->size = 0;
}

static inline void storage_remove_if(
    storage_t* storage,
    int (*predicate)(void* element, size_t id, size_t i, size_t size, void* ctx), void* ctx)
{
    for (size_t i = 0; i < storage->size; )
    {
        size_t id = storage->rindex[i];
        void* element = storage->data + i * storage->element_size;
        if (predicate(element, id, i, storage->size, ctx))
        {
            storage_delete(storage, id);
        }
        else
        {
            ++i;
        }
    }
}

static inline void storage_iter(
    storage_t* storage,
    int (*predicate)(void* element, size_t id, size_t i, size_t size, void* ctx), void* ctx)
{
    for (size_t i = 0; i < storage->size; ++i)
    {
        size_t id = storage->rindex[i];
        void* element = storage->data + i * storage->element_size;
        if (!predicate(element, id, i, storage->size, ctx))
        {
            break;
        }
    }
}

#endif