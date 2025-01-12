#include "libtesseract.h"
#include <stdlib.h>
#include <iostream>
#include "engine_one.hpp"
#include "updateBuffers.hpp"
//DynamicEngineDriver<DynamicExploreNonSym<VertexId, MotifCountingE>, MotifCountingE, UpdateBuffer>
        EngineDriver* e;
UpdateBuffer* updateBuf;
GraphUpdateType updateType;
//
// Globals
//

Algorithm algorithm;
output_callback_fun_t output_callback = NULL;

//
// Helper functions
//

EmbeddingTmp *generate_random_embeddings(const size_t num) {
    EmbeddingTmp *es = (EmbeddingTmp *)calloc(num, sizeof(EmbeddingTmp));
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
        const EmbeddingTmp *es = generate_random_embeddings(num);
        output_callback(es, num);
    }
}

//
// BEGIN NEW API
//

void init(const Configuration *configuration) {
    printf("Initialized Tesseract worker %lu (out of %lu) for algorithm %lu\n", configuration->worker_id, configuration->num_workers, configuration->algorithm_id);
    switch(configuration->algorithm_id){
//        case 100:{
//            printf("[INFO] Running external algo symetric\n");
//            if(do_updates){
//                e = new DynamicEngineDriver<DynamicExploreSymmetric<VertexId, ScalaAlgo>,ScalaAlgo,UpdateBuffer>(configuration->no_threads,true, updateBuf);
//                ( (DynamicEngineDriver<DynamicExploreSymmetric<VertexId, ScalaAlgo>, ScalaAlgo, UpdateBuffer>*)e)->getAlgo()->setAlgo(&algorithm);
//            }
//            else {
//                e = new StaticEngineDriver<StaticExploreSymmetric<VertexId, ScalaAlgo>, ScalaAlgo>(configuration->no_threads,
//                                                                                                   true);
//                ( (StaticEngineDriver<StaticExploreSymmetric<VertexId, ScalaAlgo>, ScalaAlgo>*)e)->getAlgo()->setAlgo(&algorithm);
//            }
//            break;
//        }
//        case 101:{
//            printf("[INFO] Running external algo non symmetric\n");
//            if(do_updates){
//                e = new DynamicEngineDriver<DynamicExploreNonSym<VertexId, ScalaAlgo>,ScalaAlgo,UpdateBuffer>(configuration->no_threads,false, updateBuf);
//                ( (DynamicEngineDriver<DynamicExploreNonSym<VertexId, ScalaAlgo>, ScalaAlgo,UpdateBuffer>*)e)->getAlgo()->setAlgo(&algorithm);
//            }
//            else {
//                e = new StaticEngineDriver<StaticExploreNonSym<VertexId, ScalaAlgo>, ScalaAlgo>(configuration->no_threads,
//                                                                                                   false);
//                ( (StaticEngineDriver<StaticExploreNonSym<VertexId, ScalaAlgo>, ScalaAlgo>*)e)->getAlgo()->setAlgo(&algorithm);
//            }
//            break;
//        }
//        case 0:
//        {
//            printf("[INFO] Running %d-Cliques with %d threads\n",K, configuration->no_threads);
//            if(do_updates){
////                StaticEngineDriver<StaticExploreSymmetric<VertexId , CliqueFindE>, CliqueFindE>* e_tmp = new   StaticEngineDriver<StaticExploreSymmetric<VertexId , CliqueFindE>, CliqueFindE>(configuration->no_threads,true);
////                do_updates = false;
////                e_tmp->execute_app();
////                do_updates = true;
//                e = new DynamicEngineDriver<DynamicExploreSymmetric<VertexId, CliqueFindE>,CliqueFindE,UpdateBuffer>(configuration->no_threads,true, updateBuf);
//            }
//            else
//                e = new StaticEngineDriver<StaticExploreSymmetric<VertexId,CliqueFindE>,CliqueFindE>(configuration->no_threads,true);
//            break;
//        }
//        case 1:
//        {
//            printf("[INFO] Running %d-MC with %d threads\n",K, configuration->no_threads);
//            if(do_updates) {
////                StaticEngineDriver<StaticExploreNonSym<VertexId ,MotifCountingE>, MotifCountingE>*e_tmp =  new StaticEngineDriver<StaticExploreNonSym<VertexId ,MotifCountingE>, MotifCountingE>(configuration->no_threads,false);
////                do_updates = false;
////                e_tmp->execute_app();
////                do_updates = true;
////                printf("Done executing static part\n");
////                delete(e_tmp);
//                e = new DynamicEngineDriver<DynamicExploreNonSym<VertexId, MotifCountingE>, MotifCountingE, UpdateBuffer>(
//                        configuration->no_threads, false, updateBuf);
//            }
////           else
////               e = new StaticEngineDriver<StaticExploreNonSym<VertexId,MotifCountingE>,MotifCountingE>(configuration->no_threads,false);
//            break;
//        }
//        case 2:
//        {
//            printf("[INFO] Running %d-LCliques with %d threads\n",K, configuration->no_threads);
//            if(do_updates){
//                e = new DynamicEngineDriver<DynamicExploreSymmetric<VertexId, ColorCliqueE>,ColorCliqueE,UpdateBuffer>(configuration->no_threads,true, updateBuf);
//            }
////            else
////                e = new StaticEngineDriver<StaticExploreSymmetric<VertexId,ColorCliqueE>,ColorCliqueE>(configuration->no_threads,true);
//            break;
//        }
        case 3:
        {
            printf("[INFO] Running %d-Keyword search with %d threads\n",K, configuration->no_threads);
            if(do_updates){
                e = new DynamicEngineDriver<DynamicExploreNonSym<VertexId, KSearchE>,KSearchE,UpdateBuffer>(configuration->no_threads,false, updateBuf);
            }
//            else
//                e = new StaticEngineDriver<StaticExploreNonSym<VertexId,KSearchE>,KSearchE>(configuration->no_threads,false);
            break;
        }
        default: {
            printf("You need to have a valie algo id! \n");
            exit(1);
        }
    }
}

void setGraphInputFiles(const GraphInputFiles* graphInput){
    input_file = graphInput->input_file;
    degree_file = graphInput->degree_file;
    NB_NODES  = graphInput->nb_nodes;

    init_graph_input(false); //this means that the graph is loaded in memory and offsets are not mmaped
}
void set_algorithm(const Algorithm *_algorithm) {
    algorithm.pupdate = _algorithm->pupdate;
    algorithm.pfilter = _algorithm->pfilter;
    algorithm.filter = _algorithm->filter;
    algorithm.match = _algorithm->match;
    algorithm.output = _algorithm->output;
    algorithm.init = _algorithm->init;
}

void start() {
    printf("Starting computation...\n");
    auto start = std::chrono::high_resolution_clock::now();

    e->execute_app();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "[TIME] Runtime: " << diff.count() << " seconds\n";
    printf("Finished computation!\n");
    printf("[STAT] no_active post-compute: %lu\n", no_active);
}

void stop() {

    printf("Stopping execution!\n");
    e->stop();
    if(do_updates) wait_b(&updateBuf->updates_consumed);
}
void init_update_buf(size_t b_size, size_t nb_edges, size_t nb_nodes,int no_threads, size_t initial_chunk){
    updateBuf = new UpdateBuffer(b_size,nb_edges,nb_nodes,initial_chunk,no_threads);
}

size_t preloadChunk(const size_t chunk_size, Configuration* configuration,std::vector<uint64_t>*vec){
    printf("[STAT] Preloading %lu updates\n", chunk_size);
//    std::thread** threads = (std::thread**) calloc(1, sizeof(std::thread*)); //configuration->no_threads - 1, sizeof(std::thread*));
//    for (int i = 0; i < 1; i++){///configuration->no_threads - 1; i++) {
//        threads[i] = new std::thread(&UpdateBuffer::preload_edges_before_update, updateBuf, edges_full, (i + 1), edges, 2);//(int)configuration->no_threads);
//    }
    size_t ret = updateBuf->preload_edges_before_update(edges_full, 0, edges,1,vec);// configuration->no_threads);

    printf("[INFO] Preload done\n");
//    for(int i =0; i < 1;i++){//configuration->no_threads-1; i++){
//        threads[i]->join();
//    }
//    for (int i = 0; i < configuration->no_threads - 1; i++) {
//        delete threads[i];
//    }
//    free(threads);
        return ret;
    //TODO Compute on the preloaded chunk

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
   // output_random_stuff();

    if(dst < src) return;
    updateBuf->curr_ts = ts;
    edges[adj_offsets[src] + degree[src]].src = src;
    edges[adj_offsets[src] + degree[src]].dst = dst;
    edges[adj_offsets[src] + degree[src]].ts = ts;

    degree[src]++;
    assert(degree[src]>=0);
    edges[adj_offsets[dst] + degree[dst]].src = dst;
    edges[adj_offsets[dst] + degree[dst]].dst = src;
    edges[adj_offsets[dst] + degree[dst]].ts = ts;
    degree[dst]++;
    assert(degree[dst]>=0);
    uint32_t h_src =murmur3_32(( uint8_t *)(&src), 4, dst);
//        if((true)){//

    if(true){//h_src % e->getNoWorkers()  == e->getWid()     ) {
        updateBuf->updates[updateBuf->get_no_updates()].src = src;
        updateBuf->updates[updateBuf->get_no_updates()].dst = dst;
//        updateBuf->curr_ts = ts;
        updateBuf->incNoUpdates();
    }
    //printf("Received new edge %u->%u (ts=%u)\n", src, dst, ts);
}

void edge_del(const VertexId src, const VertexId dst, const Timestamp ts) {
//    output_random_stuff();
//TODO Mark edges as deleted, decrement degree 
    uint32_t h_src =murmur3_32(( uint8_t *)(&src), 4, dst);
    if(dst < src) return;

   for(size_t i = 0; i < degree[src]; i++){
        if(edges[adj_offsets[src] + i].dst == dst) {
            edges[adj_offsets[src] + i]. ts = ts;
            break;
        }
   }

    for(size_t i = 0; i < degree[dst]; i++){
        if(edges[adj_offsets[dst] + i].dst == src) {
            edges[adj_offsets[dst] + i].ts = ts;
            break;
        }
    }
//        if((true)){//
    if(true){//h_src % e->getNoWorkers()  == e->getWid()     ) {
        updateBuf->updates[updateBuf->get_no_updates()].src = src;
        updateBuf->updates[updateBuf->get_no_updates()].dst = dst;
        updateBuf->incNoUpdates();
    }
//    printf("Received del edge %u->%u (ts=%u)\n", src, dst, ts);
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
//    printf("Received new batch with %lu entries:\n", num_entries);
    wait_b(&updateBuf->updates_consumed);
    updateBuf->resetNoUpdates() ;
    for(size_t i = 0; i < num_entries; ++i) {
        updateType = buffer[i].tpe;
//        printf("  in[%lu] = ", i);
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
//    assert(updateBuf->get_no_updates() == num_entries /2);
    wait_b(&updateBuf->updates_ready);
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
