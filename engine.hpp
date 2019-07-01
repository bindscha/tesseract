#ifndef __ENGINE_HPP__
#define __ENGINE_HPP__

#include "common_include.hpp"

#include "barrier.hpp"
#include "algo_api.hpp"
#include "graph.hpp"

//#include "clique_vector.hpp"

#include "Pattern.hpp"
#include "canonic_checks.hpp"

#include "algorithm.hpp"
#include "motif_counting.hpp"
#include "triangle_c.hpp"
#include "cliques_ooc.hpp"
#include "thread_data.hpp"
#include "updateBuffers.hpp"
#include "graph_cache.hpp"
#include <set>
#include <unordered_set>
#include <thread>
size_t curr_cliques = 0;
int K=3;
int phase = 0;

int step = 2;

#define NB_BITS 100000 //4847571
UpdateBuffer* u_buf;
uint64_t no_triangles = 0;

enum exp_mode{VERTEX, EDGE, MIDDLE};

exp_mode EXPLORE_MODE=VERTEX;
int max = K;


void printPattern( Pattern<uint32_t, edge_full>* set,uint32_t step, uint32_t v_id){

//  printf("\n");
  for(int i =0;i < step; i++){
    if(i!=0)
      printf("%u ", set->vertices_in_pattern[i]);
    else {
      uint32_t src = set->vertices_in_pattern[i]& mask;
      if(src == 0)src=set->vertices_in_pattern[i];
      else src = set->vertices_in_pattern[i] ^ mask;
      printf("%u ", src);//set->vertices_in_pattern[i]);
    }
  }
  printf("%u[%d]\n",set->vertices_in_pattern[step],set->pattern_id);

}

template<typename T>
class Engine {

    std::thread **threads;
    uint64_t* per_thread_candidates;
    GraphCache<uint32_t> filter_cache; // prefilter //one entry per node.Should be more efficient than having 1 cache with NB_NODE entries
public:
    uint64_t batch_size = 10000;
    Engine() {
      threads = (std::thread **) calloc(no_threads - 1, sizeof(std::thread *));
      filter_cache = GraphCache<uint32_t>(NB_NODES);
      assert(filter_cache.size() == NB_NODES);
    }

    ~Engine() {
      delete[] threads;
    }

    Algorithm<Pattern<uint32_t,T>>* algo;
    Pattern<uint32_t, T>* cliques_found;
    void init_active(void(*func)()){
      no_active = 0;
      func();
      per_thread_candidates = (uint64_t*) calloc(no_threads, sizeof(uint64_t));
    }
#if SYM == 1
    void explore_middleout( Pattern <uint32_t, T> *set, uint32_t step, int tid){//,const std::unordered_set<uint32_t>*ign){//}, Pattern<uint32_t, T>*ignore) {
    uint32_t dst, ts;
    uint32_t v_id = set->vertices_in_pattern[step - 1];
/*Uncomment for MO STATIC
     FOREACH_EDGE_FWD(v_id, dst)//,ts)
        if(!algo->prefilter(dst))continue;// || ts > set->ts_max  || set->is_v_in_pattern(dst, step)) continue;
//         if(ts == set->ts_max && dst < set->vertices_in_pattern[0]) continue;
//        per_thread_candidates[tid]++;
      if (!canonic_check_r2_new(dst, set, step)) continue;
        const bool should_expand = algo->expand(step);

      const uint32_t no_edges = set->no_edges_in_pattern == -1U? 0 :set->no_edges_in_pattern;
       addVertextoSet(set, dst, step);
        const bool filter = algo->filter(dst,set, step);

        if(filter){
          if(should_expand){
//            per_thread_candidates[tid]++;
            explore_middleout( set, step + 1, tid);
            }
          else{
            algo->process(set,step,tid);

            per_thread_data[tid]++;
          }
          }

        set->reset(no_edges);

      ENDFOR
      */
//    std::unordered_set<uint32_t> ignore(*ign);


      FOREACH_EDGE_TS(v_id, dst,ts)
      if(!algo->prefilter(dst) || ts> set->ts_max || set->is_v_in_pattern(dst,step)) continue;
//      if(ignore.find(dst) != ignore.end()) continue;
      if(ts == set->ts_max && dst < set->vertices_in_pattern[0]) {
//        ignore.insert(dst);
        continue;
      }

      if(step < 3 &&!canonic_check_r2_new(dst,set,step)) continue;
      if(step >= 3 && dst < v_id) continue;
//      if(step >=3 && ts == set->ts_max ) {
//           uint64_t e1 = (uint64_t)v_id << 32;
//            e1 = e1 | dst;
//            uint64_t e2 = (uint64_t)set->vertices_in_pattern[0] << 32;
//            e2 = e2 | set->vertices_in_pattern[1];
//            if(e1 < e2 ){
////             ignore.insert(dst);
//             continue;
//             }
//          }
         const bool should_expand = algo->expand(step);

         const uint32_t no_edges = set->no_edges_in_pattern == -1U? 0 :set->no_edges_in_pattern;

        addVertextoSetTS(set, dst, step);
        const bool filter = algo->filter(dst,set, step);
        if(filter){
          if(should_expand){
//          per_thread_candidates[tid]++;
            explore_middleout( set, step + 1, tid);//,&ignore);
            }
          else{
            algo->process(set,step,tid);
//            printPattern(set,step,dst);
            per_thread_data[tid]++;
          }
          }

        set->reset(no_edges);

      ENDFOR


}


#else
    void explore_middleout( Pattern <uint32_t, T> *set, uint32_t step, int tid, const  std::unordered_set<uint32_t>* neigh){//},const std::bitset<NB_BITS>*bits_ign){//std::unordered_set<uint32_t>* ign){//}, Pattern<uint32_t, T>*ignore) {
      std::unordered_set<uint32_t> neighbours(*neigh);
//      std::unordered_set<uint32_t> ignore(*ign);
//      std::bitset<NB_BITS> bits(*bits_ign);


      uint32_t dst, ts;

        uint32_t v_id = set->vertices_in_pattern[step - 1];
        FOREACH_EDGE_TS(v_id, dst, ts)
      if (!set->is_v_in_pattern(dst, step) && ts <= set->ts_max) {
        if (ts == set->ts_max &&(dst < set->vertices_in_pattern[0] )) {
//          bits.set(dst) ;
          continue;
        }//|| (v_id < set->vertices_in_pattern[0]) )  ) { bits.set(dst);continue;}

//            if(ts== set->ts_max){
//              uint64_t e1 = (uint64_t)v_id << 32;
//              e1 = e1 | dst;
//              uint64_t e2 = (uint64_t)set->vertices_in_pattern[0] << 32;
//              e2 = e2 | set->vertices_in_pattern[1];
//              if(e1 < e2 ) {
//                bits.set(dst); continue;
//              }
//            }


          neighbours.insert(dst);

        }
//        else bits.set(dst);
        ENDFOR
//      }
      for (uint32_t n: neighbours) {
//        if(ignore.find(n) != ignore.end()) continue;
//        if(bits.test(n))continue;

        if (set->is_v_in_pattern(n, step)  || !canonic_check_r2_new(n, set, step)) continue;


        const bool should_expand = algo->expand(step);

        const uint64_t no_edges = set->no_edges_in_pattern > 0 ? set->no_edges_in_pattern : 0;

        addVertextoSetTS(set, n, step);
        if(no_edges == set->no_edges_in_pattern) continue;
        const bool filter = algo->filter(n, set, step);
        if (filter)
          if (should_expand) {
            explore_middleout(set, step + 1, tid, &neighbours);//,&bits);//&ignore);
            }
            else{
              algo->process_update_tid(set, step, tid);
              per_thread_data[tid]++;
            }
            set->reset(no_edges);
          }

      }
//    }
#endif
#if SYM == 1
void explore_v(Pattern<uint32_t,T> *embedding, uint32_t step, int tid) {
      uint32_t v_id = embedding->vertices_in_pattern[step-1];

      uint32_t dst;
      FOREACH_EDGE_FWD(v_id, dst)
        if(degree[dst]<K -1){//s!algo->prefilter(dst)){//should_be_active(dst)) {

          continue;
        }
//        if(!embedding->is_v_in_pattern(dst, step) && canonic_check_r1(dst, embedding, step)){
          const uint64_t no_edges = embedding->no_edges_in_pattern > 0 ? embedding->no_edges_in_pattern : 0;
          const bool max_size_reached = algo->expand(step);//0 /* XXX: remove unused */, embedding, step);

          addVertextoSet(embedding, dst, step);

          const bool filter = algo->filter(dst, embedding, step);
          if (filter) {
            if (max_size_reached) {
//               per_thread_candidates[tid]++;
              explore_v(embedding, step + 1, tid);
            } else {
              algo->process(embedding, step, tid);
              per_thread_data[tid]++;
            }
          }
          embedding->reset(no_edges);
//        }
      ENDFOR
    }
#else
    void explore_v( Pattern <uint32_t, T> *set, uint32_t step, int tid,const std::unordered_set<uint32_t>*neigh){
      uint32_t dst;
      assert(tid <no_threads);


      std::unordered_set<uint32_t> neighbours(*neigh);

      uint32_t v_id = set->vertices_in_pattern[step-1];

        FOREACH_EDGE(v_id, dst)
        if (canonic_check_r1(dst, set, step) && !set->is_v_in_pattern(dst, step)) {
          neighbours.insert(dst);
        }
        ENDFOR

      for (uint32_t n: neighbours) {
        if(set->is_v_in_pattern(n,step) ||!canonic_check_r2(n, set, step)) continue;

        const uint32_t no_edges =set->no_edges_in_pattern > 0 ? set->no_edges_in_pattern : 0;;
        const bool filter = algo->filter(n, set, step );
        const bool expand = algo->expand(step);
        addVertextoSet(set, n, step);

        if(filter) {
          if (expand) {
            explore_v( set, step + 1, tid,&neighbours);
          } else {
            algo->process(set, step, tid);
            per_thread_data[tid]++;
          }
        }
        set->reset(no_edges);

      }

    }

#endif
    void explore(uint32_t src, Pattern <uint32_t, T> *set, uint32_t step, int tid) {


      uint32_t dst;

      for (int i = 0; i < step; i++) {
        uint32_t v_id = set->vertices_in_pattern[i];

        FOREACH_EDGE(v_id, dst)

        if (!canonic_check(dst, set, step, v_id)) {
          continue;
        }

        if (!algo->filter(v_id, set, step + 1)) continue;

        bool should_expand = algo->expand(step);


        uint64_t no_edges = set->no_edges_in_pattern;

        if (no_edges == -1U) no_edges = 0;
        addVertextoSet(set, dst, step);

        if (should_expand) {

          explore(dst, set, step + 1, tid);
        } else {
          algo->process(set,step);

          //printPattern(set,step,v_id);
//        add_item_in_buf_gen<Pattern<uint32_t, edge_full>>(cliques_found, *set, tid, &(no_active_next));
          per_thread_data[tid]++;
        }
        set->reset(no_edges);

        ENDFOR
      }
    }

    void compute_middleout(void* c){

      int tid = (long) c;
      begin:

      wait_b(&xsync_begin);

      uint64_t prev_upd = 0;
      auto start = std::chrono::high_resolution_clock::now();
      Pattern <uint32_t, T> set;
      std::bitset<NB_BITS> ignore_bit(0);
      set.alloc_vertices(max);
//      if(tid!= 0)goto end;

      while (curr_item < no_active) {
        get_work(tid, &thread_work[tid], no_active);

        if (thread_work[tid].start == thread_work[tid].stop) goto end;
//        printf("[%d]%u-%u ",tid,thread_work[tid].start,thread_work[tid].stop);
        thread_work[tid].start += u_buf->curr_batch_start;
        thread_work[tid].stop += u_buf->curr_batch_start;
        for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
          uint32_t src,dst;
          src = u_buf->edges[thread_work[tid].start].src;

          dst = u_buf->edges[thread_work[tid].start].dst;
//          printf("(%u -> %u)  ", src,dst);
          if(!algo->prefilter(src) || !algo->prefilter(dst)) continue;

#if SYM ==1
          set.reset(0);
#else
          if(src >dst)continue;
#endif
            set.vertices_in_pattern[0] = src;
            set.ts_max = u_buf->curr_ts;//edges[active[thread_work[tid].start]].ts;
            step = 1;
            addVertextoSetTS(&set, dst, step);


          std::unordered_set<uint32_t>ign;
#if SYM != 1
          std::unordered_set<uint32_t>neighbours;

          neighbours.clear();
//          ignore_bit.reset();

          uint32_t d,ts;
          FOREACH_EDGE_TS(src, d,ts)
          if(d != dst && ts <= set.ts_max) {
            if (ts == set.ts_max && (d < src)) {
//              ignore_bit.set(d);
              continue;
            }
            neighbours.insert(d);
          }
          ENDFOR
//                  FOREACH_EDGE_TS(dst, d,ts)
//
//          if (ts == set.ts_max && d < src) {ignore_bit.set(d);}
//
//
//          ENDFOR
          explore_middleout( &set, 2, tid,&neighbours);//,&ignore_bit);//,&neighbours);

#else

//          ign.clear();
          uint32_t d,ts;
//          FOREACH_EDGE_TS(src, d,ts)
//          if(d != dst && ts <= set.ts_max) {
//            if (ts == set.ts_max && (d < src || d < dst)) { ign.insert(d);}
//
////            neighbours.insert(d);
//          }
//          ENDFOR
//           FOREACH_EDGE_TS(dst, d,ts)
//
//            if (ts == set.ts_max && d < src) { ign.insert(d);}
//
//
//          ENDFOR
////          assert(set.no_edges_in_pattern != 0);
          explore_middleout( &set, 2, tid);//,&ign);
#endif
          set.reset(0);

        }
        if(thread_work[tid].start % 100000 == 0)
          printf("Processed %u\n", thread_work[tid].start );
      }
      end:
      auto end = std::chrono::high_resolution_clock::now();

      std::chrono::duration<double, std::milli> diff = end - start;
//      printf("%d %.3f\n", tid, diff.count());
      if (thread_bufs[tid].curr != 0) {
        //flush buf
      }

      wait_b(&xsync_end);

      if (tid != 0) goto begin;
    }
    void compute(void *c) {

      int tid = (long) c;
      assert(tid<no_threads);

      begin:

      wait_b(&xsync_begin);

      uint64_t prev_upd = 0;
      auto start = std::chrono::high_resolution_clock::now();

      Pattern <uint32_t, T> set;

      set.alloc_vertices(max);
      uint32_t no_edges = -1U;
#if SYM== 1
      if(set.no_edges_in_pattern == -1U) set.no_edges_in_pattern  =0;
#endif
      std::unordered_set<uint32_t> neighbours = std::unordered_set<uint32_t>();

//      if(tid!= 0) goto end;
      while (curr_item <no_active) {

        get_work(tid, &thread_work[tid], no_active);

        if (thread_work[tid].start == thread_work[tid].stop) goto end;

        for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
          uint32_t src = edges[active[thread_work[tid].start]].src;

          uint32_t dst = edges[active[thread_work[tid].start]].dst;
#if SYM != 1
          if(dst <src ) continue;
#else
//          if(!algo->prefilter(dst)) continue;
#endif

          set.vertices_in_pattern[0] = src;

          uint32_t step = 1;




#if SYM == 1
            uint32_t d;
            set.reset(0);
             addVertextoSet(&set, dst, step);
             no_edges = 0;
             explore_v( &set, step+1, tid);

#else
            uint32_t d;
            neighbours.clear();
            FOREACH_EDGE(src, d)
            if(canonic_check_r1(d, &set, step)  && d!= dst)
              neighbours.insert(d);

            ENDFOR
            addVertextoSet(&set, dst, step);
          if (no_edges == -1U) no_edges = 0;
          explore_v( &set, step+1, tid, &neighbours);
          set.reset(no_edges);
#endif
//          set.reset(no_edges);


//    if(per_thread_data[tid] - prev_upd != 0) printf("%lu %u %.3f\n", per_thread_data[tid] - prev_upd,  src,diff.count());//src<<" "<< diff.count() << "\n";
          prev_upd = per_thread_data[tid];
          if(thread_work[tid].start % 100000 == 0)
            printf("Processed %u\n", thread_work[tid].start );

        }
      }
      end:
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> diff = end - start;
      printf("%d %.3f\n", tid, diff.count());


      wait_b(&xsync_end);
      if (tid != 0) goto begin;
    }

    Pattern<uint32_t , T>* per_thread_cliques;

    void execute_app() {

//      per_thread_cliques = (Pattern<uint32_t, T>*)malloc(no_threads * sizeof(Pattern<uint32_t, T>));


      if(EXPLORE_MODE == MIDDLE){
        for (int i = 0; i < no_threads - 1; i++) {
          threads[i] = new std::thread(&Engine<T>::compute_middleout, this, (void *) (i + 1));
        }
      }
      else {
        for (int i = 0; i < no_threads - 1; i++) {
          threads[i] = new std::thread(&Engine<T>::compute, this, (void *) (i + 1));
        }
      }

      if(!do_updates){
        curr_item = 0;
        uint64_t cand = 0;
        curr_cliques = 0;
        compute(0);
        curr_item = 0;

        no_triangles = 0;
        for (int i = 0; i < no_threads; i++) {
          no_triangles += per_thread_data[i];
          per_thread_data[i] = 0;
        }
        printf("Found %lu\n",no_triangles);
        return;
      }

      u_buf = new UpdateBuffer(batch_size , edges_full, NB_EDGES, NB_NODES );
      free (edges_full);
      uint64_t items_processed = 0;
      float total_time = 0;
      do {
        curr_item = 0;
        uint64_t cand = 0;
        curr_cliques = 0;

//        u_buf->set_batch_size();

        u_buf->update_graph_structure(edges);
        no_active = u_buf->curr_batch_end - u_buf->curr_batch_start;
        printf("\n[BATCH %lu - %lu ](%u): ",u_buf->curr_batch_start,u_buf->curr_batch_end,no_active);
        auto start = std::chrono::high_resolution_clock::now();

          compute_middleout(0);

        auto end = std::chrono::high_resolution_clock::now();
        u_buf->batch_size=1000000;
        std::chrono::duration<double, std::milli> diff = end - start;
        if(items_processed != 0)
          total_time += diff.count()/1000.;
              printf("done in %.3f\n",  diff.count()/1000.);
        no_active = no_active_next;
        phase = 1 - phase;
        curr_item = 0;

        no_triangles = 0;
        for (int i = 0; i < no_threads; i++) {
          no_triangles += per_thread_data[i];
          per_thread_data[i] = 0;
        }
        items_processed += no_triangles;
        printf("Found %lu\n",no_triangles);

      }while(u_buf->curr_batch_end < u_buf->no_edges);
      uint64_t no_checks =0 ;
      for (int i = 0; i < no_threads; i++) {
        no_checks += per_thread_candidates[i];
        per_thread_candidates[i] = 0;
      }
      printf("Total discovered %lu in %.3f s\n",items_processed, total_time);

    }

    void cancel_compute() {
      for (int i = 0; i < no_threads - 1; i++) {
//        pthread_cancel(threads[i]);
      }
    }



};
#endif