#ifndef __LIBTESSERACT_HPP__
#define __LIBTESSERACT_HPP__

#include <stdlib.h>
#include <stdint.h>
#include<vector>
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
    int no_threads;
} Configuration;

extern "C" typedef struct{
    char* input_file;
    char* degree_file;
    size_t nb_nodes;
}GraphInputFiles;

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
} EmbeddingTmp;

typedef void (*init_fun_ptr_t)();
typedef bool (*pfilter_fun_ptr_t)(const EmbeddingTmp *embedding, const VertexId vertexId);
typedef void (*pupdate_fun_ptr_t)();
typedef bool (*filter_fun_ptr_t)(const EmbeddingTmp *embedding);
typedef bool (*match_fun_ptr_t)(const EmbeddingTmp *embedding);
typedef void (*output_fun_ptr_t)(const EmbeddingTmp *pre_embedding, const EmbeddingTmp *post_embedding);
typedef void (*output_single_fun_ptr_t)(const EmbeddingTmp *pre_embedding);

typedef struct {
    init_fun_ptr_t init;
    pfilter_fun_ptr_t pfilter;
    pupdate_fun_ptr_t pupdate;
    filter_fun_ptr_t filter;
    match_fun_ptr_t match;
    output_fun_ptr_t output;
    output_single_fun_ptr_t output_single;
} Algorithm;

extern "C" typedef void (*output_callback_fun_t)(const void *buffer, const size_t num_entries);

extern "C" void init(const Configuration *configuration);

extern "C" void setGraphInputFiles(const GraphInputFiles* graphInput);

extern "C" void set_algorithm(const Algorithm *algorithm);

extern "C" void start();

extern "C" void stop();

extern "C" size_t preloadChunk(const size_t chunk_size, Configuration* configuration,std::vector<uint64_t>*vec);

extern "C" void vertex_new(const VertexId id, const Timestamp ts);

extern "C" void vertex_del(const VertexId id, const Timestamp ts);

extern "C" void edge_new(const VertexId src, const VertexId dst, const Timestamp ts);

extern "C" void edge_del(const VertexId src, const VertexId dst, const Timestamp ts);

extern "C" void vertex_label_is(const VertexId id, const char *key, const void *value, const Timestamp ts);

extern "C" void edge_label_is(const VertexId src, const VertexId dst, const char *key, const void *value, const Timestamp ts);

extern "C" void batch_new(const GraphUpdate *buffer, size_t num_entries);

extern "C" void set_output_callback(output_callback_fun_t f);

extern "C" void unset_output_callback();
extern Algorithm algorithm;
extern GraphUpdateType updateType;
//TODO Add functions for initializing the update buffer structure
extern output_callback_fun_t output_callback;
extern "C" void init_update_buf(size_t b_size, size_t nb_edges, size_t nb_nodes, int no_threads, size_t initial_chunk);
//
// END NEW API
//

#endif
