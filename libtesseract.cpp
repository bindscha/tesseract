#include "libtesseract.h"

#include <stdlib.h>
#include <iostream>

//
// Globals
//

Algorithm algorithm;
output_callback_fun_t output_callback = NULL;

//
// Helper functions
//

Embedding *generate_random_embeddings(const size_t num) {
    Embedding *es = (Embedding *)calloc(num, sizeof(Embedding));
    for (size_t i = 0; i < num; ++i) {
        size_t num_vertices = rand() % MAX_EMBEDDING_SIZE;
        for (size_t j = 0; j < num_vertices; ++j) {
            es[i].vertices[j] = rand();
        }
        es[i].num_vertices = num_vertices;
        es[i].ts = i;
        es[i].edges_mat = rand() % 32;
        es[i].ts_mat = rand() % 32;
        es[i].status = (OutputStatus) (rand() % 3);
    }
    return es;
}

void output_random_stuff() {
    if(output_callback != NULL) {
        const size_t num = 1 + rand() % 10;
        const Embedding *es = generate_random_embeddings(num);
        output_callback(es, num);
    }
}

//
// BEGIN NEW API
//

void init(const Configuration *configuration) {
    printf("Initialized Tesseract worker %lu (out of %lu) for algorithm %lu\n", configuration->worker_id, configuration->num_workers, configuration->algorithm_id);
}

void set_algorithm(const Algorithm *algorithm) {

}

void start() {
    printf("Started execution!\n");
}

void stop() {
    printf("Stopped execution!\n");
}

void vertex_new(const VertexId id, const Timestamp ts) {
    output_random_stuff();
    printf("Received new vertex %u (ts=%u)\n", id, ts);
}

void vertex_del(const VertexId id, const Timestamp ts) {
    output_random_stuff();
    printf("Received del vertex %u (ts=%u)\n", id, ts);
}

void edge_new(const VertexId src, const VertexId dst, const Timestamp ts) {
    output_random_stuff();
    printf("Received new edge %u->%u (ts=%u)\n", src, dst, ts);
}

void edge_del(const VertexId src, const VertexId dst, const Timestamp ts) {
    output_random_stuff();
    printf("Received del edge %u->%u (ts=%u)\n", src, dst, ts);
}

void vertex_label_is(const VertexId id, const char *key, const void *value, const Timestamp ts) {
    output_random_stuff();
    printf("Received label %s='%s' for vertex %u (ts=%u)\n", key, (const char *)value, id, ts);
}

void edge_label_is(const VertexId src, const VertexId dst, const char *key, const void *value, const Timestamp ts) {
    output_random_stuff();
    printf("Received label %s='%s' for edge %u->%u (ts=%u)\n", key, (const char *)value, src, dst, ts);
}

void batch_new(const GraphUpdate *buffer, size_t num_entries) {
    printf("Received new batch with %lu entries:\n", num_entries);
    for(size_t i = 0; i < num_entries; ++i) {
        printf("  in[%lu] = ", i);
        switch(buffer[i].tpe) {
            case VertexAdd:
                vertex_new(buffer[i].src, buffer[i].ts);
                break;
            case VertexDel:
                vertex_del(buffer[i].src, buffer[i].ts);
                break;
            case EdgeAdd:
                edge_new(buffer[i].src, buffer[i].dst, buffer[i].ts);
                break;
            case EdgeDel:
                edge_del(buffer[i].src, buffer[i].dst, buffer[i].ts);
                break;
            case VertexLabelMod:
                vertex_label_is(buffer[i].src, buffer[i].key, (char *)buffer[i].value, buffer[i].ts);
                break;
            case EdgeLabelMod:
                edge_label_is(buffer[i].src, buffer[i].dst, buffer[i].key, (char *)buffer[i].value, buffer[i].ts);
                break;
            default:
                printf("Unknown update type %u! Ignored...\n", buffer[i].tpe);
        }
    }
}

void set_output_callback(output_callback_fun_t f) {
    output_callback = f;
    printf("Output callback set!\n");
}

void unset_output_callback() {
    output_callback = NULL;
    printf("Output callback unset!\n");
}

//
// END NEW API
//
