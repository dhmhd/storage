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
    size_t cur_id;
    uint8_t* swap_place;
} storage_t;

static inline int storage_init(
    storage_t* storage,
    size_t initial_capacity,
    size_t element_size)
{
    storage->element_size = element_size;
    storage->capacity = initial_capacity;
    if (storage->capacity == 0)
    {
        storage->capacity = 1;
    }
    storage->size = 0;
    storage->cur_id = 0;
    storage->data = (uint8_t*)malloc(storage->capacity * element_size);
    storage->swap_place = (uint8_t*)malloc(element_size);
    storage->rindex = NULL;
    storage->index = NULL;
    return storage->data != NULL;
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
        size_t idx = storage->size;
        *ret_id = storage->rindex[idx];
        return 1;
    }
    size_t new_id = storage->cur_id;
    size_t new_cur_id = storage->cur_id + 1;
    size_t new_len = new_cur_id * sizeof(size_t);
    size_t* new_rindex = (size_t*)realloc(storage->rindex, new_len);
    if (!new_rindex)
    {
        return 0;
    }
    size_t* new_index = (size_t*)realloc(storage->index, new_len);
    if (!new_index)
    {
        return 0;
    }
    storage->rindex = new_rindex;
    storage->index = new_index;
    storage->rindex[new_id] = new_id;
    storage->index[new_id] = storage->size;
    storage->cur_id = new_cur_id;
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