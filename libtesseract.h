#ifndef __LIBTESSERACT_HPP__
#define __LIBTESSERACT_HPP__

#include <stdlib.h>
#include <stdint.h>

//
// BEGIN NEW API
//

#define MAX_KEY_LENGTH 8
#define MAX_EMBEDDING_SIZE 8

typedef size_t WorkerId;
typedef size_t AlgorithmId;

typedef uint32_t VertexId;
typedef uint32_t Timestamp;
typedef uint32_t AdjacencyMatrix;

extern "C" typedef struct {
    WorkerId worker_id;
    uint64_t num_workers;
    AlgorithmId algorithm_id;
} Configuration;

extern "C" typedef enum { VertexAdd = 0, VertexDel = 1, EdgeAdd = 2, EdgeDel = 3, VertexLabelMod = 4, EdgeLabelMod = 5 } GraphUpdateType;

// Laurent: I am defining it this way rather than using a union -- slightly unclean, but union makes a big mess
extern "C" typedef struct {
    VertexId src;
    VertexId dst;
    Timestamp ts;
    char key[MAX_KEY_LENGTH];
    void *value;
    GraphUpdateType tpe;
} GraphUpdate;

extern "C" typedef enum { Add = 0, Del = 1, Mod = 2 } OutputStatus;

extern "C" typedef struct {
    VertexId vertices[MAX_EMBEDDING_SIZE];
    size_t num_vertices;
    Timestamp ts;
    AdjacencyMatrix edges_mat;
    AdjacencyMatrix ts_mat;
    OutputStatus status;
} Embedding;

typedef void (*init_fun_ptr_t)();
typedef void (*pfilter_fun_ptr_t)(const Embedding *embedding, const VertexId vertexId);
typedef void (*pupdate_fun_ptr_t)();
typedef void (*filter_fun_ptr_t)(const Embedding *embedding);
typedef void (*match_fun_ptr_t)(const Embedding *embedding);
typedef void (*output_fun_ptr_t)(const Embedding *pre_embedding, const Embedding *post_embedding);

typedef struct {
    init_fun_ptr_t init;
    pfilter_fun_ptr_t pfilter;
    pupdate_fun_ptr_t pupdate;
    filter_fun_ptr_t filter;
    match_fun_ptr_t match;
    output_fun_ptr_t output;
} Algorithm;

extern "C" typedef void (*output_callback_fun_t)(const void *buffer, const size_t num_entries);

extern "C" void init(const Configuration *configuration);

extern "C" void set_algorithm(const Algorithm *algorithm);

extern "C" void start();

extern "C" void stop();

extern "C" void vertex_new(const VertexId id, const Timestamp ts);

extern "C" void vertex_del(const VertexId id, const Timestamp ts);

extern "C" void edge_new(const VertexId src, const VertexId dst, const Timestamp ts);

extern "C" void edge_del(const VertexId src, const VertexId dst, const Timestamp ts);

extern "C" void vertex_label_is(const VertexId id, const char *key, const void *value, const Timestamp ts);

extern "C" void edge_label_is(const VertexId src, const VertexId dst, const char *key, const void *value, const Timestamp ts);

extern "C" void batch_new(const GraphUpdate *buffer, size_t num_entries);

extern "C" void set_output_callback(output_callback_fun_t f);

extern "C" void unset_output_callback();

//
// END NEW API
//

#endif
