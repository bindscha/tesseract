#ifndef __EMBEDDING_HPP__
#define __EMBEDDING_HPP__

#include <array>
#include <iostream>
#include <set>
#include <stdint.h>
#include "graph.hpp"


#define EDGE_TIMESTAMPS

#define MAX_SIZE 8
typedef uint32_t Edges;
typedef uint32_t Timestamp;


// Triangular numbers
uint32_t T_N[] = {0, 0, 1, 3, 6, 10, 15, 21, 28};
uint32_t T_N_MASK[] = {0, 0, 0b1, 0b111, 0b111111, 0b1111111111, 0b111111111111111, 0b111111111111111111111, 0b1111111111111111111111111111};

// Computed with the following python script
//MAX = 8
//
//T_N = [0, 0, 1, 3, 6, 10, 15, 21, 28]
//
//def compute_edge_mask(idx):
//        e = 0
//        for i in range(0, idx):
//                bit = T_N[idx] + i
//                e |= 1 << bit
//        for i in range(idx+1, MAX):
//                bit = T_N[i] + idx
//                e |= 1 << bit
//        return e
//
//def compute_edge_mask_format(idx):
//        return "{0:b}".format(compute_edge_mask(idx))
//
//for idx in range(0, MAX):
//        print('edge mask for %d: %s' % (idx, compute_edge_mask_format(idx)))
uint32_t T_N_MASK_EDGES[] = {0b1000001000010001001011, 0b10000010000100010010101, 0b100000100001000100100110, 0b1000001000010001000111000, 0b10000010000100001111000000, 0b100000100000111110000000000, 0b1000000111111000000000000000, 0b1111111000000000000000000000};


template<typename V>
class Embedding {

private:
    V vertices_[MAX_SIZE] = 0;
    Edges edges_;

    size_t no_vertices_;

#ifdef EDGE_TIMESTAMPS
    Timestamp max_ts_;
    Edges timestamps_;
#endif

    inline size_t _edge_bit(size_t src_idx, size_t dst_idx) {
        assert(src_idx != 0 && src_idx != dst_idx);
        return 1 << (T_N[src_idx] + dst_idx);
    }

    inline void _append(V vertex) {
//        assert(no_vertices_ < MAX_SIZE);
        vertices_[no_vertices_++] = vertex;
    }

#ifdef EDGE_TIMESTAMPS
    inline void _connect(size_t src_idx, size_t dst_idx, Timestamp ts) {
        if (src_idx == 0 && dst_idx == 1 || src_idx == 1 && dst_idx == 0) {
            max_ts_ = ts;
        }
        if (src_idx > dst_idx) {
            edges_ |= _edge_bit(src_idx, dst_idx);
            if (ts == max_ts_) {
                timestamps_ |= _edge_bit(src_idx, dst_idx);
            }
        } else if (src_idx < dst_idx) {
            edges_ |= _edge_bit(dst_idx, src_idx);
            if (ts == max_ts_) {
                timestamps_ |= _edge_bit(dst_idx, src_idx);
            }
        }
    }
#else
    inline void _connect(size_t src_idx, size_t dst_idx) {
        if (src_idx > dst_idx) {
            edges_ |= _edge_bit(src_idx, dst_idx);
        } else if (src_idx < dst_idx) {
            edges_ |= _edge_bit(dst_idx, src_idx);
        }
    }
#endif

    inline bool _contains_edge(size_t src_idx, size_t dst_idx) {
        if (src_idx > dst_idx) {
            return edges_ & _edge_bit(src_idx, dst_idx);
        } else if (src_idx < dst_idx) {
            return edges_ & _edge_bit(dst_idx, src_idx);
        } else {
            return false;
        }
    }

#ifdef EDGE_TIMESTAMPS
    inline bool _contains_edge_timestamp(size_t src_idx, size_t dst_idx) {
        if (src_idx > dst_idx) {
            return timestamps_ & _edge_bit(src_idx, dst_idx);
        } else if (src_idx < dst_idx) {
            return timestamps_ & _edge_bit(dst_idx, src_idx);
        } else {
            return false;
        }
    }
#endif

    inline void _add_edges() {
        if(no_vertices_ > 1) {
            V dst;
#ifdef EDGE_TIMESTAMPS
            Timestamp ts;
            FOREACH_EDGE_TS(vertices_[no_vertices_-1], dst, ts)
#else
            FOREACH_EDGE(vertices_[no_vertices_-1], dst)
#endif
            for(size_t i = 0; i < no_vertices_; ++i) {
                if (vertices_[i] == dst) {
#ifdef EDGE_TIMESTAMPS
                    _connect(no_vertices_-1, i, ts);
#else
                    _connect(no_vertices_-1, i);
#endif
                    break;
                }
            }
            ENDFOR
        }
    }

    inline size_t _find_index(V vertex) {
        for (size_t i = 0; i < no_vertices_; ++i) {
            if (vertices_[i] == vertex) {
                return i;
            }
        }
        return -1;
    }

public:
    Embedding(): vertices_({0}), edges_(0), no_vertices_(0) {
#ifdef EDGE_TIMESTAMPS
        max_ts_ = 0;
        timestamps_ = 0;
#endif
    };

    Embedding(V vertex): vertices_({0}), edges_(0), no_vertices_(1) {
#ifdef EDGE_TIMESTAMPS
        max_ts_ = 0;
        timestamps_ = 0;
#endif
        vertices_[0] = vertex;
    };

    Embedding(V vertex1, V vertex2): vertices_({0}), edges_(0), no_vertices_(2) {
        vertices_[0] = vertex1;
        vertices_[1] = vertex2;
#ifdef EDGE_TIMESTAMPS
        max_ts_ = 0;
        timestamps_ = 0;
        _add_edges();
#else
        _connect(0, 1);
#endif
    };

    Embedding(const Embedding& other): vertices_({0}), edges_(other.edges_), no_vertices_(other.no_vertices_) {
        memcpy(vertices_, other.vertices_, no_vertices_ * sizeof(V));
#ifdef EDGE_TIMESTAMPS
        max_ts_ = other.max_ts_;
        timestamps_ = other.timestamps_;
#endif
    }

    bool operator==(Embedding other) {
        // Laurent: following Jasmina's definition of ==
        return no_edges() == other.no_edges();
    }
    Edges getEdges() const{
        return edges_;
    }
    void operator=(Embedding other) {
        memcpy(vertices_, other.vertices_, MAX_SIZE * sizeof(V));
        edges_ = other.edges_;
        no_vertices_ = other.no_vertices_;
#ifdef EDGE_TIMESTAMPS
        max_ts_ = other.max_ts_;
        timestamps_ = other.timestamps_;
#endif
    }

    V& operator[] (size_t idx) {
        return vertices_[idx];
    }

    const V& operator[] (size_t idx) const {
        return vertices_[idx];
    }

    inline void append(V vertex) {
        _append(vertex);
        _add_edges();
    }

    inline void append_no_edges(V vertex) {
        _append(vertex);
    }

    inline void fill_in_edges() {
        _add_edges();
    }

    inline size_t no_vertices() const {
        return no_vertices_;
    }

    inline size_t no_edges() const {
        return __builtin_popcount(edges_);
    }

#ifdef EDGE_TIMESTAMPS
    inline const Timestamp max_ts() const{
        return max_ts_;
    }
#endif

    inline const V first() {
        if (no_vertices_ > 0) {
            return vertices_[0];
        } else {
            return 0;
        }
    }

    inline const V last(){
        if (no_vertices_ > 0) {
            return vertices_[no_vertices_ - 1];
        } else {
            return 0;
        }
    }

    inline const size_t vertex_degree_at_index(size_t idx) const {
        if (idx >= 0 && idx < no_vertices_) {
            return __builtin_popcount(edges_ & T_N_MASK_EDGES[idx]);
        } else {
            return 0;
        }
    }

    inline const size_t vertex_degree(V vertex) {
        size_t idx = _find_index(vertex);
        if (idx != -1) {
            return vertex_degree_at_index(idx);
        } else {
            return 0;
        }
    }

    inline void pop() {
        if (no_vertices_ == 0) {
            return;
        }
        vertices_[--no_vertices_] = 0;
        edges_ &= T_N_MASK[no_vertices_];
#ifdef EDGE_TIMESTAMPS
        timestamps_ &= T_N_MASK[no_vertices_];
        if (no_vertices_ < 2) {
            max_ts_ = 0;
        }
#endif
    }

    inline void pop(size_t k) {
        if (k > no_vertices_) {
            k = no_vertices_;
        }
        for (size_t i = 0; i < k; ++i) {
            pop();
        }
    }

    inline void truncate(size_t k) {
        pop(no_vertices_ > k ? no_vertices_ - k : 0);
    }

    inline const bool contains(V vertex) {
        return _find_index(vertex) != -1;
    }

    inline const bool contains_edge_at_indices(size_t src_idx, size_t dst_idx) {
        return _contains_edge(src_idx, dst_idx);
    }

    inline const bool contains_edge(V src, V dst) {
        size_t src_idx = _find_index(src);
        size_t dst_idx = _find_index(dst);
        if (src_idx != -1 && dst_idx != -1) {
            return contains_edge_at_indices(src_idx, dst_idx);
        } else {
            return false;
        }
    }

#ifdef EDGE_TIMESTAMPS
    inline const size_t old_vertex_degree_at_index(size_t idx) const {
        if (idx >= 0 && idx < no_vertices_) {
            return __builtin_popcount((edges_ ^ timestamps_) & T_N_MASK_EDGES[idx]);
        } else {
            return 0;
        }
    }

    inline const size_t old_vertex_degree(V vertex) {
        size_t idx = _find_index(vertex);
        if (idx != -1) {
            return old_vertex_degree_at_index(idx);
        } else {
            return 0;
        }
    }

    inline const bool edge_at_indices_is_new(size_t src_idx, size_t dst_idx) {
        return _contains_edge_timestamp(src_idx, dst_idx);
    }

    inline const bool edge_is_new(V src, V dst) {
        size_t src_idx = _find_index(src);
        size_t dst_idx = _find_index(dst);
        if (src_idx != -1 && dst_idx != -1) {
            return edge_at_indices_is_new(src_idx, dst_idx);
        } else {
            return false;
        }
    }
#endif

    inline void print() {
        fprintf(stderr, "%lu-embedding [ ", no_vertices());
        for (size_t i = 0; i < MAX_SIZE; ++i) {
            fprintf(stderr, "%lu ", vertices_[i]);
        }
        fprintf(stderr, "]");
        fprintf(stderr, " with %lu updates: ", no_edges());
        for (size_t i = 1; i < no_vertices_; ++i) {
            for (size_t j = 0; j < i; ++j) {
                if (_contains_edge(i, j)) {
                    fprintf(stderr, "%lu->%lu ", vertices_[i], vertices_[j]);
#ifdef EDGE_TIMESTAMPS
                    fprintf(stderr, "(%s) ", edge_at_indices_is_new(i, j) ? "NEW" : "OLD");
#endif
                }
            }
        }
        fprintf(stderr, "\n");
        fflush(stdout);
    }
};

#endif
