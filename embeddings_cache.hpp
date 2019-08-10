#ifndef __EMBEDDINGS_CACHE_HPP__
#define __EMBEDDINGS_CACHE_HPP__

#include <array>
#include <iostream>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <set>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "uthash.h"

#include "embedding.hpp"


const size_t DEFAULT_CACHE_SIZE = 400000000;

/* hash of hashes */
typedef struct item {
    uint32_t id;
    struct item *sub;
    Embedding<uint32_t> val;
    UT_hash_handle hh;
} item_t;


item_t **items_ = NULL;



template<typename E>
class EmbeddingsCache {
private:
    size_t num_vertices_;
    size_t num_entries_;
    size_t capacity_;

    inline item_t *new_item(uint32_t id, Embedding<uint32_t> *val) {
        item_t *i = (item_t *) malloc(sizeof(item_t));
        i->val = *val;
        i->sub = NULL;
        i->id = id;
        return i;
    }

public:
    EmbeddingsCache(): num_vertices_(0), num_entries_(0), capacity_(DEFAULT_CACHE_SIZE) {};

    EmbeddingsCache(size_t num_vertices, size_t capacity): num_vertices_(num_vertices), num_entries_(0), capacity_(capacity) {
        items_ = (item_t **) calloc(num_vertices_, sizeof(item_t *));
    };

    ~EmbeddingsCache() {
        free(items_);
    }

    inline item_t * operator[](const size_t idx) const{
        return items_[idx];
    }

    inline item_t * line(size_t idx) {
        return items_[idx];
    }

    inline size_t size() {
        return num_vertices_;
    }

    inline size_t num_entries() const{
        return num_entries_;
    }

    inline uint32_t get_value_at_line(uint32_t line, uint32_t idx) {
        item_t *i = get_item_at_line(line, idx);
        if (i == NULL) {
            return -1;
        } else {
            return NULL;
        }
    }

    inline item_t *get_item_at_line(const uint32_t line, const uint32_t idx) const {
        item_t *i = NULL;
        HASH_FIND_INT(items_[line], &idx, i);
        return i;
    }

    inline void insert(uint32_t line, uint32_t i, uint32_t j, Embedding<uint32_t> *val) {
        item_t *root = NULL;
        HASH_FIND_INT(items_[line], &i, root);
        if (root == NULL) {
            root = new_item(i, val);
            HASH_ADD_INT(items_[line], id, root);
        }

        item_t *replaced = NULL;
        item_t *child = new_item(j, val);
        HASH_REPLACE_INT(root->sub, id, child, replaced);
        if(replaced != NULL) {
            free((void *) replaced);
        } else {
            ++num_entries_;

        }
    }

};


#endif