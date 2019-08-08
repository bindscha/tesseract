//
// Created by jasmi on 6/25/2019.
//

#ifndef TESSERACT_ENGINE_ONE_HPP
#define TESSERACT_ENGINE_ONE_HPP
#include <bitset>
#include "common_include.hpp"
#include "barrier.hpp"
#include "algo_api.hpp"
#include "embedding.hpp"
#include "graph.hpp"
#include "embeddings_cache.hpp"
#include "graph_cache.hpp"
#include "canonic_checks.hpp"
//#include "triangle_c.hpp"
#include "cliques_ooc.hpp"
#include "scala_algos.h"
#include "motif_counting.hpp"
#include "color_cliques.hpp"
//#include "updateBuffers.hpp"
//#include "thread_data.hpp"
#include "filter_cache.hpp"
#include <set>
#include <unordered_set>
#include <thread>
#include <mutex>
#include"Bitmap.h"

uint64_t CHUNK_SIZE= 1;
struct thread_work_t{
    uint32_t start;
    uint32_t stop;
};
size_t* per_thread_data;
size_t curr_item = 0;
// const size_t NB_BITS = 4847571;//100000;//61578414;//105896554;



inline  void get_work(int tid, thread_work_t* t_work, uint32_t max){

    uint32_t incr = CHUNK_SIZE;
//  if( num == 0) incr = 1;
    uint32_t idx = __sync_fetch_and_add(&curr_item, incr);

    if(idx >=max) {t_work->start = t_work->stop = max; return;}

    t_work->start = idx;

    t_work->stop = t_work->start + incr;
    if(t_work->stop >max )t_work->stop = max;
}
template <typename T,typename A>
class StaticExploreSymmetric{
    A algo;

    inline void explore_sym(Embedding<T> *embedding,const int step,const int tid){
        VertexId v_id = embedding->last();
        VertexId dst;
        FOREACH_EDGE_FWD(v_id, dst)
            if(algo.pattern_filter(embedding,dst)) continue;
            embedding->append(dst);
            //TODO Missing call to R2 check. Will work for Cliques but might not for other algos
            const bool filter =embedding->no_edges()  == ((embedding->no_vertices())* (embedding->no_vertices() -1 ))/2; algo.filter(embedding);
            if(filter) {
                if(step < K -1 ){
                    explore_sym(embedding, step + 1, tid);
                }
                else {
                    per_thread_data[tid]++;
//                    printf("ST (");
//                    for(int k = 0; k < K ; k++){
//                        printf("%u ",(*embedding)[k]);
//                    }
//                    printf(")\n");
                }
//                if(algo.match(embedding)){
////                    algo.output(embedding);
//                    per_thread_data[tid]++;
//                }
//
//                if (step < K - 1) {
//                    explore_sym(embedding, step + 1, tid);
//                }
            }
            embedding->pop();
        ENDFOR
    }
public:
    StaticExploreSymmetric(){ }

    void explore(Embedding<T> *embedding, int step, const int tid, const std::unordered_set<VertexId> *neigh,
                 const std::unordered_set<VertexId> *ign)  {
        explore_sym(embedding,step,tid);
    }
    inline void  buildNeighbours(const Embedding<VertexId > embedding,const VertexId src,const VertexId  dst, std::unordered_set<VertexId>* neighbours){

    }

};
template<typename T, typename A>
class StaticExploreNonSym{
    A algo;
    void explore_nonSym(Embedding<T>* embedding, int step, const int tid, const std::unordered_set<VertexId>* neigh) {
        std::unordered_set<VertexId> neighbours(*neigh);
        const VertexId v_id = embedding->last();
        VertexId dst;
        FOREACH_EDGE(v_id, dst)
            if(embedding->first() < dst && !embedding->contains(dst))
                neighbours.insert(dst);

        ENDFOR

        for(VertexId n:neighbours){
            if(embedding->contains(n) || !canonic_check_r2E_nonsym(n, embedding,step)) continue;
            //TODO pattern_filter check - not needed for motifs but might be for other algorithm
            embedding->append(n);
            const bool filter = algo.filter(embedding);
            if(filter){
                if(algo.match(embedding)){
                    algo.output(embedding,tid);
                    per_thread_data[tid]++;
                }
                if(step < K-1)
                    explore_nonSym(embedding, step + 1, tid, &neighbours);
            }
//            if(step < K - 1)
//                explore_nonSym(embedding, step + 1, tid, &neighbours);
//            else{
//                // TODO change the following to :this->algorithm->match(embedding);
//                uint32_t deg[K];
//                for(int i = 0; i < embedding->no_vertices(); i++){
//                    deg[i] = embedding->vertex_degree_at_index(i);
//                }
//                std::sort(deg, deg+ K);
//                int pattern_id1 = 0;
//                int i = 0;
//                for(const auto &item: deg){
//                    pattern_id1 = pattern_id1 | (int)item;
//                    if(i == embedding->no_vertices() - 1 )break;
//                    i++;
//                    pattern_id1 = pattern_id1 << 2;
//                }
//                per_thread_patterns[tid][pattern_id1]++;
//                //                                    algorithm->process(embedding, step, tid);
//                per_thread_data[tid]++;
//
//            }
            embedding->pop();
        }
    }

public:
    StaticExploreNonSym() {}
    void explore(Embedding<T> *embedding, int step,const int tid, const std::unordered_set<VertexId>* neigh=NULL, const std::unordered_set<VertexId>* ign=NULL){
        explore_nonSym(embedding,step, tid, neigh);
    }
   inline void buildNeighbours(const Embedding<VertexId > embedding,const VertexId src,const VertexId  dst, std::unordered_set<VertexId>* neighbours){
        neighbours->clear();
        VertexId d;
        FOREACH_EDGE(src, d)
            if(canonic_check_r1E(d, &embedding, 1)  && d!= dst)
                neighbours->insert(d);
        ENDFOR
    }

};

template<typename T, typename A>
class DynamicExploreSymmetric{
    A algo;
    bool e_cache_enabled = false;//true;
    int no_threads;
    EmbeddingsCache<VertexId> *e_cache;

    std::vector<Embedding<VertexId>> *per_thread_e_cache_buffer;

//    void exploreSym(Embedding<VertexId> *embedding, int step, const int tid){//},const std::bitset<NB_BITS>*ign) { //const std::unordered_set<VertexId>* ign){
//
////        delete bits_ign;
//    }
    void sort3(uint32_t& a, uint32_t& b, uint32_t& c)
    {
        if (a > b)
        {
            std::swap(a, b);
        }
        if (b > c)
        {
            std::swap(b, c);
        }
        if (a > b)
        {
            std::swap(a, b);
        }
    }
public:
    DynamicExploreSymmetric(int n_threads)  {
        no_threads = n_threads;
        e_cache_enabled = false;// true;
        if(e_cache_enabled) {
            e_cache = new EmbeddingsCache<uint32_t>(NB_NODES, DEFAULT_CACHE_SIZE);
            per_thread_e_cache_buffer = new std::vector<Embedding<uint32_t>>[no_threads];
            for(size_t i = 0; i < no_threads; ++i) {
                per_thread_e_cache_buffer[i].reserve(100000);
            }
        }
    }
    ~DynamicExploreSymmetric(){
        if (e_cache != NULL) {
            delete e_cache;
        }
    }
    inline void toggle_cache() {
        e_cache_enabled = !e_cache_enabled;
        printf("Cache is: %d\n", e_cache_enabled);
    }

    void explore(Embedding<VertexId> *embedding, int step,const int tid, const  std::unordered_set<VertexId>* neigh=NULL){//, const  std::unordered_set<VertexId>*ign=NULL){//const std::bitset<NB_BITS>*ign= NULL){//const std::unordered_set<VertexId>* ign=NULL){
        VertexId dst, v_id = embedding->last();
        Timestamp ts;
//          std::unordered_set<VertexId>bits_ign(*bit_in);
//       std::bitset<NB_BITS>*bits_ign = new std::bitset<NB_BITS> (*ign);
//        std::unordered_set<VertexId> ignore(*ign);
        uint64_t e1 = (uint64_t) ((*embedding)[0]) << 32;
        e1 = e1 | (uint64_t) ((*embedding)[1]);

        FOREACH_EDGE_TS(v_id, dst, ts)

            if (!algo.pattern_filter(embedding, dst))continue;
            if (step >= 3 && dst < v_id) {continue;}
            if (ts == embedding->max_ts() && dst < embedding->first()) { continue;}
            if (ts > embedding->max_ts() || embedding->contains(dst))continue;



            embedding->append(dst);


            bool cont = false;

            for (int k = 0; k < embedding->no_vertices() - 1; k++) {

//                if((*embedding)[k] == v_id) continue;
                if (embedding->edge_at_indices_is_new(embedding->no_vertices() - 1, k)) {
//                    printf("Edge %u - %u is new\n", (*embedding)[k], dst);

//                    uint64_t e1 = dst > v_id ? v_id : dst;// (uint64_t)dst << 32;
//                    e1 = (e1 << 32) | (dst > v_id ? dst : v_id);

//                        uint64_t e1 = (uint64_t) ((*embedding)[0]) << 32;
//                        e1 = e1 | (uint64_t) ((*embedding)[1]);

                    uint64_t src1 = (uint64_t) ((*embedding)[k]);
                    uint64_t e2 = (dst > src1 ? src1 : dst);
                    e2 = (e2 << 32);
                    e2 = e2 | (uint64_t) (dst > src1 ? dst : src1);
                    if (e2 < e1) {
                        embedding->pop();
                        cont = true;
                        break;
                    }
                }
            }
            if (cont) continue;

            if ( step < 3 && !canonic_check_r2_middle(dst, embedding)) {
                embedding->pop();
                continue;
            }


            bool hit = false;
            if (e_cache_enabled && step == 2) {
                item_t *entries = e_cache->get_item_at_line(embedding->first(), dst);
                if (entries != NULL) {
                    hit = true;
                    item_t *item1, *tmp1;
                    HASH_ITER(hh, entries->sub, item1, tmp1)
                    {
                        Embedding<uint32_t> *cached_embedding = &(item1->val);
                        cached_embedding->append(v_id);
                        const bool filter = embedding->no_edges() == ((step + 2) + step) /
                                                                     2;//   .filter(cached_embedding);//, step + 1); //TODO this will probably need to be fixed
                        if (filter) {
                            per_thread_data[tid]++;
                        }
                        cached_embedding->pop();
                    }
                }
            }

            if (hit) {
                embedding->pop();
                continue;
            }

            const bool filter = embedding->no_edges() ==
                                ((embedding->no_vertices()) * (embedding->no_vertices() - 1)) /
                                2;//algo.filter(embedding);
            if (filter) {
                if (step < K - 1) {
                    explore(embedding, step + 1, tid);//, &bits_ign);//&ignore);
                    if (e_cache_enabled) {
                        per_thread_e_cache_buffer[tid].push_back(*embedding);
                    }
                } else {
                    per_thread_data[tid]++;

//                    printf("[%u - %u] {",v_id, dst);
//                    for(int i = 0 ; i  < embedding->no_vertices();i++){
//                        printf(" %u ", (*embedding)[i]);
//                    }
//                    printf("}\n");
                }

//                if(algo.match(embedding)){
////                    algo.output(embedding);
//                    per_thread_data[tid]++;
//                }
//                if(step < K -1){
//                    exploreSym(embedding, step + 1, tid, &ignore);
//                    if (e_cache_enabled) {
//                        per_thread_e_cache_buffer[tid].push_back(*embedding);
//                    }
//                }
            }
            embedding->pop();
        ENDFOR


//        exploreSym(embedding,step,tid);//, ign);//ign);
        }
        inline void updateCaches(){
            if(e_cache_enabled) {
                for (size_t i = 0; i < no_threads; ++i) {
                    //fprintf(stderr, "Updating cache for thread %lu with %lu items...", i, per_thread_e_cache_buffer[i].size());
                    for (size_t j = 0; j < per_thread_e_cache_buffer[i].size(); ++j) {
                        uint32_t first = per_thread_e_cache_buffer[i][j][0], second = per_thread_e_cache_buffer[i][j][1], third = per_thread_e_cache_buffer[i][j][2];
                        sort3(first, second, third);
                        e_cache->insert(first, second, third, &per_thread_e_cache_buffer[i][j]);
                    }
                    per_thread_e_cache_buffer[i].clear();
                }
                //fprintf(stderr, "Cache contains %lu entries!\n", e_cache->num_entries());
            }
    }
};
template<typename T, typename A>
class DynamicExploreNonSym {
//    static const size_t NB_BITS = 100000;
    A algo;
    int no_threads; //TODO not used
public:

    DynamicExploreNonSym(int n_threads) :no_threads(n_threads)  {}
    inline void updateCaches(){}
    void explore(Embedding<VertexId>* embedding, int step, const int tid,const  std::unordered_set<VertexId>* neigh=NULL){//}, const  std::unordered_set<VertexId>* bits_in=NULL){//const std::bitset<NB_BITS>*ign= NULL){//},  const std::unordered_set<VertexId>* ign=NULL){
        std::unordered_set<VertexId> neighbours(*neigh);
//        std::unordered_set<uint32_t> ignore(*ign);
//        std::bitset<NB_BITS>bits_ign(*ign);
//        std::unordered_set<VertexId>bits_ign(*bits_in);


        VertexId dst;
        Timestamp ts;
        const VertexId v_id = embedding->last();
        FOREACH_EDGE_TS(v_id, dst, ts)
//            if (!algo.pattern_filter(embedding,dst))continue;
            if(!embedding->contains(dst) && ts <= embedding->max_ts()){
                if(ts == embedding->max_ts() && dst < embedding->first()){
//                    bits_ign.insert(dst);
//                    ignore.insert(dst);
                    continue;
                }
            }

            if(ts == embedding->max_ts()){
                uint64_t e1 = dst>v_id?v_id : dst;// (uint64_t)dst << 32;
                e1 =(e1 << 32 ) |(dst >v_id? dst:v_id);


                uint64_t e2 = (uint64_t)((*embedding)[0]) << 32;
                e2 = e2 | (uint64_t)((*embedding)[1]);
                if(e1 < e2 ){
//                    bits_ign.insert(dst);
//                    ignore.insert(dst);
                    continue;
                }
            }
            neighbours.insert(dst);
        ENDFOR

        for(VertexId n: neighbours){
//            if(bits_ign.find(n) != bits_ign.end()) continue;
//            if(ignore.find(n) != ignore.end()) continue; // TODO this is can_expand in the paper
            if(embedding->contains(n) || !canonic_check_r2_middle(n, embedding)) continue;
            embedding->append(n);
            bool cont = false;
            for(int k = 0; k < embedding->no_vertices() -1; k++){
//                if((*embedding)[k] == v_id) continue;
                if(embedding->edge_at_indices_is_new(embedding->no_vertices() -1,k)){
//                    printf("Edge %u - %u is new\n", (*embedding)[k], dst);

//                    uint64_t e1 = dst > v_id ? v_id : dst;// (uint64_t)dst << 32;
//                    e1 = (e1 << 32) | (dst > v_id ? dst : v_id);

                    uint64_t e1 = (uint64_t) ((*embedding)[0]) << 32;
                    e1 = e1 | (uint64_t) ((*embedding)[1]);

                    uint64_t src1 = (uint64_t) ((*embedding)[k]);
                    uint64_t e2 = (dst > src1? src1 : dst);
                    e2 = (e2 << 32);
                    e2 = e2 | (uint64_t) (dst > src1 ? dst : src1);
                    if (e2 < e1) {
                        embedding->pop();
                        cont = true;
                        break;
                    }
                }
            }
            if(cont) { continue;}
            const bool filter = true;//algo.filter(embedding);
            if(filter ){
//                if(algo.match(embedding)) {
//                    algo.output(embedding, tid);
//                    per_thread_data[tid]++;
//                }
                if(step < K -1)
                    explore(embedding, step + 1 , tid, &neighbours);//, &bits_ign);//ignore);
                else {
                    per_thread_data[tid]++;
                    std::array<uint32_t,K> deg;
                    std::array<uint32_t,K> deg2;


                    int no_edg = 0;
                    for(int i = 0; i < embedding->no_vertices(); i++){
                        deg[i] = embedding->vertex_degree_at_index(i);
#ifdef EDGE_TIMESTAMPS
                        deg2[i] = embedding->old_vertex_degree_at_index(i);
                        no_edg += embedding->old_vertex_degree_at_index(i);
#endif
                    }
//        std::sort(deg, deg + K);


                    std::sort(deg.begin(), deg.end());

                    int pattern_id1 = 0;
                    int i = 0;

                    for(const auto &item: deg){

                        pattern_id1 = pattern_id1 | (int)item;

                        if(i == embedding->no_vertices() - 1 )break;
                        i++;
                        pattern_id1 = pattern_id1 << 2;
                    }
                    int pattern_id2= 0;
                    i = 0;
                    if(no_edg/2 < (embedding->no_vertices()-1)) pattern_id2 = 0;
                    else {
                        std::sort(deg2.begin(), deg2.end());
                        for (const auto &item: deg2) {

                            if (item == 0) {
                                pattern_id2 = 0;
                                break;
                            }
                            pattern_id2 = pattern_id2 | (int) item;

                            if (i == embedding->no_vertices() - 1)break;
                            i++;
                            pattern_id2 = pattern_id2 << 2;
                        }
                    }
                    per_thread_patterns[tid][pattern_id1]++;

                    if (pattern_id2 != 0 && pattern_id2 != pattern_id1) {
                        per_thread_patterns[tid][pattern_id2]--;
                    }

//                    int pattern_id1 = 0;
//                    int i = 0;
//
//                    for(const auto &item: deg){
//
//                        pattern_id1 = pattern_id1 | (int)item;
//
//                        if(i == embedding->no_vertices() - 1 )break;
//                        i++;
//                        pattern_id1 = pattern_id1 << 2;
//                    }
//
////                    if((updateType == GraphUpdateType::EdgeDel) && do_updates) {
////                        per_thread_patterns[tid][pattern_id1]--;
////                    }
////                    else{
//                        per_thread_patterns[tid][pattern_id1]++;
////                    }
////                    if(!do_updates || GraphUpdateType::EdgeDel == updateType) return;
//                    int pattern_id2= 0;
//                    i = 0;
//                    if(no_edg/2 < (embedding->no_vertices()-1)) pattern_id2 = 0;
//                    else {
////            std::sort(deg2, deg2 + K);
//                        std::sort(deg2.begin(), deg2.end());
//
//                        for (const auto &item: deg2) {
//
//                            if (item == 0) {
//                                pattern_id2 = 0;
//                                break;
//                            }
//                            pattern_id2 = pattern_id2 | (int) item;
//
//                            if (i == embedding->no_vertices() - 1)break;
//                            i++;
//                            pattern_id2 = pattern_id2 << 2;
//                        }
//                    }
//
//                    if (pattern_id2 != 0 && pattern_id2 != pattern_id1) {
//                        per_thread_patterns[tid][pattern_id2]--;
//                    }

//                    algo.output(embedding, tid);
                }
                }

                embedding->pop();
            }
        }

};


class EngineDriver{
public:
    virtual void stop(){

    }


    EngineDriver(){


    }

    ~EngineDriver(){

    }
    virtual void execute_app(){

    }

};

template<typename E, typename A>
class StaticEngineDriver: public EngineDriver{
    bool symmetric = true;
    std::thread** threads;
    int no_workers = 1;
    int w_id = 0;
    x_barrier xsync_begin, xsync_end;
    thread_work_t *thread_work;
    int no_threads;


    E* exploreEngine;
    A algo;
    size_t edges_processed = 0;
    void compute(void* c) {
        int tid = (long) c;
        begin:
        wait_b(&xsync_begin);
        size_t prev_upd = 0;
        auto start = std::chrono::high_resolution_clock::now();

        Embedding<VertexId> embedding;
        while (curr_item < no_active) {
            get_work(tid, &thread_work[tid], no_active);

//            __sync_fetch_and_add(&edges_processed, (thread_work[tid].stop - thread_work[tid].start));
            if (thread_work[tid].start == thread_work[tid].stop) goto end;
            for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {

                VertexId src, dst;

                src = edges[active[thread_work[tid].start]].src;
                dst = edges[active[thread_work[tid].start]].dst;
//                if(src == 0 || dst == 0)
//                    continue;
                if (dst < src) continue;
//                if(!algo.pattern_filter(&embedding,src) || !algo.pattern_filter(&embedding,src)) continue;
                if(!algo.pattern_filter(&embedding,src)) continue;
                embedding.append(src);
                if( !algo.pattern_filter(&embedding,dst)) {
                    embedding.pop();
                    continue;
                }

                std::unordered_set <VertexId> neighbours; //I tried to make this a f() but too much overhead
                if(!symmetric) {
                    neighbours.clear();
                    VertexId d;
                    FOREACH_EDGE(src, d)
                        if (d > src && d != dst)
                            neighbours.insert(d);
                    ENDFOR
                }

//                embedding.append(src);
                embedding.append(dst);

                exploreEngine->explore(&embedding, 2, tid, &neighbours,NULL);

                embedding.pop();
                embedding.pop();

                if ((thread_work[tid].start <= 100000 && thread_work[tid].start % 10000 == 0) ||
                    (thread_work[tid].start <= 1000000 && thread_work[tid].start % 100000 == 0) ||
                    thread_work[tid].start % 1000000 == 0) {
                    printf("Processed %u / %lu\n", thread_work[tid].start, no_active);
                }

            }
        }
      end:
        wait_b(&xsync_end);

        if (tid != 0 ) goto begin;

        printf("Threads processed %lu edges\n",edges_processed);
    }
public:
    inline int getNoWorkers(){
        return no_workers;
    }
    inline int getWid(){
        return w_id;
    }
    StaticEngineDriver(int nb_threads, bool symm, int wid=0, int noWorker =1):no_threads(nb_threads){
        symmetric = symm;
        exploreEngine=  new E();
        threads = (std::thread**) calloc(no_threads - 1, sizeof(std::thread *));
        init_barrier(&xsync_begin, no_threads);
        init_barrier(&xsync_end, no_threads);
        per_thread_data = (size_t *) calloc(no_threads, sizeof(size_t));
        thread_work = (thread_work_t *) calloc(no_threads, sizeof(thread_work_t));

        symmetric = symm;
        w_id = wid;
        no_workers = noWorker;
        printf("[INFO] Finished engine init_graph_input\n");
    }
    A* getAlgo(){
        return &algo;
    }
    void stop(){

    }
    ~StaticEngineDriver(){
        for(int i = 0; i < no_threads; i++)
            delete threads[i];
        free(threads);
        free(per_thread_data);
        free(thread_work);
    }

    void execute_app(){
        algo.init();
        printf("[STAT] Number of active items: %lu\n",no_active);

        for (int i = 0; i < no_threads - 1; i++) {
            this->threads[i] = new std::thread(&StaticEngineDriver::compute, this, (void *) (i + 1));
        }

        curr_item  = 0;
        uint64_t cand = 0;
        compute(0);
        curr_item = 0;
       size_t no_triangles = 0;
        for (int i = 0; i < no_threads; i++) {
            no_triangles += per_thread_data[i];
            per_thread_data[i] = 0;
        }
        algo.setItemsFound(no_triangles);
//        printf("[INFO Driver] Found %lu\n",no_triangles);
        algo.output_final();
    }
};

template<typename E, typename A, typename U>
class DynamicEngineDriver: public EngineDriver {
    bool symmetric = true;
    std::thread** threads;
    int no_workers = 1;
    int w_id = 0;
    x_barrier xsync_begin, xsync_end;
    thread_work_t *thread_work;
    int no_threads;
    bool do_run = true;
    E *exploreEngine;
    A algo;
    U *uBuf;
    size_t items_processed =0 ;
//    static const size_t NB_BITS = 10000;
    void compute(void*c){
        int tid = (long) c;
        begin:

        wait_b(&xsync_begin);
//        std::unordered_set<VertexId>ign;
        Embedding<uint32_t> embedding;
//         std::bitset<NB_BITS>ign (0);
//         std::unordered_set<VertexId> ign;

        while(curr_item < no_active){
            get_work(tid, &thread_work[tid], no_active);
            if(thread_work[tid].start == thread_work[tid].stop) goto end;

            for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
                VertexId  src,dst;
                src = uBuf->updates[thread_work[tid].start].src;
                dst = uBuf->updates[thread_work[tid].start].dst;

//                if(dst< src) continue;
                if(degree[src] < K -1 || degree[dst] < K - 1)continue;
//                if(!algo.pattern_filter(&embedding,src) )continue;
//                if(!algo.pattern_filter(&embedding,dst)) continue;
//                if(!algo.pattern_filter(&embedding,src)) continue;
                embedding.append(src);
//                if(!algo.pattern_filter(&embedding, dst)){ embedding.pop(); continue;}// || !algo.pattern_filter(&embedding, src)) continue;

//                embedding.append(src);
                embedding.append(dst);


//                if(!algo.filter(&embedding)){
//                    embedding.pop();
//                    embedding.pop();
//                    continue;
//                }
//                ign.clear();
//                ign.clear();

                std::unordered_set<VertexId > neighbours;
//                neighbours.clear();


                    VertexId d;
                    Timestamp ts;
//                    bool skip = false;
                if(!symmetric){
//                    FOREACH_EDGE_TS(src, d,ts)
//                        if(d != dst && ts <= embedding.max_ts()) {
//                            if (ts == embedding.max_ts() && (d < dst)) { skip =true;break;}// ign.insert(d);}//ign.insert(d);}
//                        }
//                    ENDFOR
//                        FOREACH_EDGE_TS(dst, d, ts)
//                            //            if(d == src) continue;
//                            if (ts == embedding.max_ts() &&  d < src ) { skip =true; break;}//ign.insert(d);}//ign.insert(d);}
//                        ENDFOR

//                        if(skip) { embedding.pop(); embedding.pop(); continue;}
//                }
//                else{
                    neighbours.clear();
                    FOREACH_EDGE_TS(src, d, ts)
                        if (d != dst && ts <= embedding.max_ts()) {
                            if (ts == embedding.max_ts() && (d < src)) {
//                                ign.insert(d);
//                                ign.insert(d);
                                continue;
                            }
                            neighbours.insert(d);
                        }
                    ENDFOR
                }
                exploreEngine->explore(&embedding, 2, tid, &neighbours);//, &ign);

                embedding.pop();
                embedding.pop();
            }
        }
        end:
        wait_b(&xsync_end);
        if (tid != 0) goto begin;
    }

public:
    inline int getNoWorkers(){
        return no_workers;
    }
    inline int getWid(){
        return w_id;
    }
    DynamicEngineDriver(int nb_threads, bool symm, U* uB, int wid = 0,  int noWorker =1):no_threads(nb_threads){
        symmetric = symm;
        exploreEngine=  new E(nb_threads);
        threads = (std::thread**) calloc(no_threads - 1, sizeof(std::thread *));
        init_barrier(&xsync_begin, no_threads);
        init_barrier(&xsync_end, no_threads);
        per_thread_data = (size_t *) calloc(no_threads, sizeof(size_t));
        thread_work = (thread_work_t *) calloc(no_threads, sizeof(thread_work_t));

        symmetric = symm;
        w_id = wid;
        no_workers = noWorker;

        exploreEngine=  new E(nb_threads);
        uBuf = uB;
    }
    ~DynamicEngineDriver(){
        for(int i = 0; i < no_threads; i++)
            delete threads[i];
        free(threads);
        free(per_thread_data);
        free(thread_work);
    }
    A* getAlgo(){
        return &algo;
    }
    inline void stop(){
        do_run = false;
    }
    void execute_app(){
        for (int i = 0; i < no_threads - 1; i++) {
            this->threads[i] = new std::thread(&DynamicEngineDriver::compute, this, (void *) (i + 1));
        }
        printf("[INFO] Running in symmetric mode %d \n", symmetric);
        curr_item = 0;
        uint64_t cand = 0;
        size_t no_batches =0;
        wait_b(&uBuf->updates_consumed);
        double total_time = 0;
        while(do_run){
            wait_b(&uBuf->updates_ready);
            no_active = uBuf->get_no_updates();

            curr_item = 0;
            auto start = std::chrono::high_resolution_clock::now();
            compute(0);
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> diff = end - start;
            std::cout << "[TIME] Batch process time: " << diff.count() << " seconds\n";
            total_time += diff.count();
            no_active = no_active_next;
           size_t no_triangles = 0;
            for (int i = 0; i < no_threads; i++) {
                no_triangles += per_thread_data[i];
                per_thread_data[i] = 0;
            }
            size_t no_batches =0;
            items_processed += no_triangles;
//            if(no_batches > (NB_EDGES / no_active) / 2){
//                CHUNK_SIZE = 4;
//            }
            algo.setItemsFound(no_triangles);
            algo.output_final();
            exploreEngine->updateCaches();
//            printf("[STAT] Found %lu (total %lu) \n",no_triangles, items_processed);
            wait_b(&uBuf->updates_consumed);
        }
        printf("[TIME] Total Algo time %.3f\n", total_time);

    }


};

//from the interface
#endif //TESSERACT_ENGINE_ONE_HPP
