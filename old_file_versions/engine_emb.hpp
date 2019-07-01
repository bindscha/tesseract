#ifndef __ENGINE_EMB__
#define __ENGINE_EMB__
#include "common_include.hpp"

#include "barrier.hpp"
#include "algo_api.hpp"


#include "utils.hpp"
#include "graph.hpp"
#include "embedding.hpp"
//#include "clique_vector.hpp"

#include "Pattern.hpp"
#include "graph_cache.hpp"
#include "embeddings_cache.hpp"
#include "canonic_checks.hpp"

#include "algorithm.hpp"
#include "motif_counting.hpp"
#include "triangle_c.hpp"
#include "cliques_ooc.hpp"
#include "color_cliques.hpp"
#include "updateBuffers.hpp"
#include "thread_data.hpp"
#include "filter_cache.hpp"
#include <set>
#include <unordered_set>
#include <thread>
#include <mutex>

//int K= 3;

size_t curr_cliques = 0;

int phase = 0;

int step = 2;

#define NB_BITS 100000 //4847571
UpdateBuffer* u_buf;
uint64_t no_triangles = 0;

enum exp_mode{VERTEX, EDGE, MIDDLE};

exp_mode EXPLORE_MODE=VERTEX;
int max = K;


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


template<typename T,typename A>
class Engine{

   std::thread **threads;
   uint64_t *per_thread_candidates;
   EmbeddingsCache<uint32_t> *e_cache;
   std::vector<Embedding<uint32_t>> *per_thread_e_cache_buffer;
   bool e_cache_enabled;
   FilterCache <uint32_t,A> filter_cache;
   uint64_t initial_chunk = 0;
   uint64_t batch_size = 10000;
    int w_id = 0;
   int no_workers = 1;
   // prefilter //one entry per node.Should be more efficient than having 1 cache with NB_NODE entries
   public:

//   Algorithm<T>
           A* algo;

   void init_active(){
      no_active = 0;
      algo->activate_nodes();

   }
   Engine() {
      threads = (std::thread **) calloc(no_threads - 1, sizeof(std::thread *));
      //      filter_cache = FilterCache<uint32_t>(2, NB_NODES, algo);

      //      assert(filter_cache.size() == NB_NODES);
   }
   Engine(size_t b_size, size_t _initial_chunk, A* a, int _w_id, int _no_workers): batch_size(b_size), initial_chunk(_initial_chunk),filter_cache(2, NB_NODES, a), w_id(_w_id), no_workers(_no_workers){
      printf("NB_NODES %lu\n",NB_NODES);
      threads = (std::thread **) calloc(no_threads - 1, sizeof(std::thread *));
     per_thread_candidates = (uint64_t*) calloc(no_threads, sizeof(uint64_t));
      algo = a;

      e_cache_enabled = true;
      if(e_cache_enabled) {
          e_cache = new EmbeddingsCache<uint32_t>(NB_NODES, DEFAULT_CACHE_SIZE);
          per_thread_e_cache_buffer = new std::vector<Embedding<uint32_t>>[no_threads];
          for(size_t i = 0; i < no_threads; ++i) {
              per_thread_e_cache_buffer[i].reserve(100000);
          }
      }
   }

   ~Engine() {
      delete[] threads;
      if (e_cache != NULL) {
          delete e_cache;
      }

   }
   void init(){
      //load_graph
      init();
      init_thread_d_gen<Embedding<T>>(clique_fd);
   }




#if SYM == 1
   void explore_v(T *embedding, uint32_t step, int tid) {
      uint32_t v_id = embedding->last();
      uint32_t dst;
      FOREACH_EDGE_FWD(v_id, dst)
         if(!algo->prefilter(embedding,dst)) continue;
      //         if(degree[dst] < K-1){
      //            continue;
      //         }
      embedding->append(dst);

               const bool filter =algo->filter(dst,embedding,step);//
//      const bool filter = embedding->no_edges() == ((step+1)* (step))/2;
      if (filter) {
         if (step<K - 1) {
            //               per_thread_candidates[tid]++;
            explore_v(embedding, step + 1, tid);
         } else {
            //              algo->process(embedding, step, tid);
            per_thread_data[tid]++;
         }
      }
      embedding->pop();
      ENDFOR
   }
   void explore_middleout( Embedding <uint32_t> *embedding, uint32_t step, const int tid,const std::unordered_set<uint32_t>*ign){
      uint32_t dst, ts;
      uint32_t v_id = embedding->last();
      std::unordered_set<uint32_t> ignore(*ign);
      FOREACH_EDGE_TS(v_id, dst, ts)
     if(!algo->prefilter(embedding, dst)) continue;

        bool hit = false;
          if(e_cache_enabled && step == 2) {
              item_t *entries = e_cache->get_item_at_line(embedding->first(), dst);
              if(entries != NULL) {
                  hit = true;
                  item_t *item1, *tmp1;
                  HASH_ITER(hh, entries->sub, item1, tmp1) {
                      Embedding<uint32_t> *cached_embedding = &(item1->val);
                      cached_embedding->append(v_id);
                      const bool filter = algo->filter(dst, cached_embedding, step + 1);
                      if (filter) {
                          per_thread_data[tid]++;
                      }
                      cached_embedding->pop();
                  }
              }
          }

        if(hit) continue;

        if (ts > embedding->max_ts() || embedding->contains(dst)) continue;
        if (ignore.find(dst) != ignore.end()) continue;
        if (ts == embedding->max_ts() && dst < embedding->first()) {
            ignore.insert(dst);
            continue;
        }

        if (step < 3 && !canonic_check_r2_new<uint32_t>(dst, embedding, step)) continue;
        if (step >= 3 && dst < v_id) continue;
        if (step >= 3 && ts == embedding->max_ts()) {
            uint64_t e1 = dst > v_id ? v_id : dst;// (uint64_t)dst << 32;
            e1 = (e1 << 32) | (dst > v_id ? dst : v_id);
            uint64_t e2 = (uint64_t)((*embedding)[0]) << 32;
            e2 = e2 | (uint64_t)((*embedding)[1]);
            if (e1 < e2) {
                ignore.insert(dst);
                continue;
            }
        }
        embedding->append(dst);

        const bool filter = algo->filter(dst, embedding, step);//
//      const bool filter = embedding->no_edges() == ((step+1)* (step))/2; //
        if (filter) {
            if (step < K - 1) {
                //          per_thread_candidates[tid]++;
                explore_middleout(embedding, step + 1, tid, &ignore);
                if (e_cache_enabled) {
                    per_thread_e_cache_buffer[tid].push_back(*embedding);
                }
            } else {
                per_thread_data[tid]++;
            }
        }

        embedding->pop();

      ENDFOR


   }
#else
   void explore_v( Embedding <uint32_t> *embedding, uint32_t step, int tid,const std::unordered_set<uint32_t>*neigh){
      uint32_t dst;
      std::unordered_set<uint32_t> neighbours(*neigh);

      const uint32_t v_id = embedding->last();

      FOREACH_EDGE(v_id, dst)
         if (embedding->first() < dst && !embedding->contains(dst)){//canonic_check_r1E(dst, embedding, step) && !embedding->contains(dst)){
            neighbours.insert(dst);
         }
         ENDFOR

            for (uint32_t n: neighbours) {
               if(embedding->contains(n) ||!canonic_check_r2E(n, embedding, step)) continue;

               //        const uint32_t no_edges =embedding->no_edges_in_pattern > 0 ? set->no_edges_in_pattern : 0;;
               const bool filter = true;//algo->filter(n, set, step );
               //        const bool expand = algo->expand(step);
               embedding->append(n);

               //        if(filter) {
               if (step <K-1) {
                  explore_v( embedding, step + 1, tid,&neighbours);
               } else {
                  std::array<uint32_t,K> deg;
                  for(int i = 0; i < embedding->no_vertices(); i++){
                     deg[i] = embedding->vertex_degree_at_index(i);
                  }
                  std::sort(deg.begin(),deg.end());
                  int pattern_id1 = 0;
                  int i = 0;
                  for(const auto &item: deg){
                     pattern_id1 = pattern_id1 | (int)item;
                     if(i == embedding->no_vertices() - 1 )break;
                     i++;
                     pattern_id1 = pattern_id1 << 2;
                  }
                  per_thread_patterns[tid][pattern_id1]++;
                  //                                    algo->process(embedding, step, tid);
                  per_thread_data[tid]++;
               }
               //        }
               embedding->pop();

            }

         }

      void explore_middleout(Embedding<uint32_t> *embedding, uint32_t step,const int tid, const  std::unordered_set<uint32_t>* neigh,const std::bitset<NB_BITS>*bits_ign){//std::unordered_set<uint32_t>* ign){//}, Pattern<uint32_t, T>*ignore) {
         std::unordered_set<uint32_t> neighbours(*neigh);
         //      std::unordered_set<uint32_t> ignore(*ign);
         std::bitset<NB_BITS> bits(*bits_ign);


         uint32_t dst, ts;

         const uint32_t v_id = embedding->last();
         FOREACH_EDGE_TS(v_id, dst, ts)
            if (!embedding->contains(dst) && ts <= embedding->max_ts()) {
               if (ts ==  embedding->max_ts() &&(dst < embedding->first())) {
                  bits.set(dst) ;
                  continue;
               }//|| (v_id < set->vertices_in_pattern[0]) )  ) { bits.set(dst);continue;}

            if(ts== embedding->max_ts()){
               uint64_t e1 = dst>v_id?v_id : dst;// (uint64_t)dst << 32;
               e1 =(e1 << 32 ) |(dst >v_id? dst:v_id);


               uint64_t e2 = (uint64_t)((*embedding)[0]) << 32;
               e2 = e2 | (uint64_t)((*embedding)[1]);
               if(e1 < e2 ){
                  bits.set(dst);
                  continue;
               }
            }
            neighbours.insert(dst);
            }
         //        else bits.set(dst);
         ENDFOR
            //      }
            for (uint32_t n: neighbours) {
               //        if(ignore.find(n) != ignore.end()) continue;
               if(bits.test(n))continue;

               if (embedding->contains(n) || !canonic_check_r2_new(n, embedding, step)) continue;
               embedding->append(n);


               //        if(no_edges == set->no_edges_in_pattern) continue;
               const bool filter =true;// algo->filter(n, set, step);
               if (filter)
                  if (step <K-1) {

                     explore_middleout(embedding, step + 1, tid, &neighbours,&bits);//&ignore);
                  }
                  else{
                     //            algo->process_update_tid(embedding, step, tid);
                     ///************BEGIN PROCESS
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
                     std::sort(deg.begin(),deg.end());

                     //            int pattern_id1 = 0;
                     //            int i = 0;
                     //            int pattern_id2 = 0;
                     //            int j = 0;
                     //            bool to_break = false;

                     //            if(deg2[0] == 0) { pattern_id2 = 0; to_break = true;}
                     //            for(int i =0; i <embedding->no_vertices();i++){
                     //              if(!to_break) {
                     //                pattern_id2 = pattern_id2 | (int)deg2[i];
                     //                no_edg += deg2[i];
                     //                if(i == embedding->no_vertices() - 1 )break;
                     //                pattern_id2 = pattern_id2 << 2;
                     //              }
                     //              pattern_id1 = pattern_id1 | (int)deg;
                     //
                     //              if(i == embedding->no_vertices() - 1 )break;
                     //              pattern_id1 = pattern_id1 << 2;
                     //            }
                     //            if(no_edg/2 < (embedding->no_vertices()-1)) pattern_id2 = 0;
                     //
                     //            per_thread_patterns[tid][pattern_id1]++;
                     //
                     //            if (pattern_id2 != 0 && pattern_id2 != pattern_id1) {
                     //              per_thread_patterns[tid][pattern_id2]--;
                     //            }
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
                     //*************END PROCESS
                     per_thread_data[tid]++;

                  }
               embedding->pop();
            }

      }
#endif

      void compute_middleout(void*c){
         int tid = (long) c;
begin:

         wait_b(&xsync_begin);

         uint64_t prev_upd = 0;
         auto start = std::chrono::high_resolution_clock::now();
#if SYM == 1
         std::unordered_set<uint32_t>ign;
#else
         std::bitset<NB_BITS> ignore_bit(0);
#endif
         Embedding<uint32_t> embedding;
         while (curr_item <no_active) {

            get_work(tid, &thread_work[tid], no_active);

            if (thread_work[tid].start == thread_work[tid].stop) goto end;
            for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {

//           for(size_t i = 0 ; i <u_buf->get_no_updates();i++){
               uint32_t src, dst;

//               src = u_buf->edges[i].src;
//                dst = u_buf->edges[i].dst;
              src = u_buf->edges[thread_work[tid].start].src;
                dst = u_buf->edges[thread_work[tid].start].dst;

//             uint32_t h_src =murmur3_32(( uint8_t *)(&src), 4, dst);
//            if(!(h_src % no_workers == tid )) continue;
//             printf("TID %d processing %u (%u-%u)\n",tid,i,src,dst);
//             __sync_fetch_and_add(&curr_item,1);
               //                    if(phase == 0){
               //                      filter_cache.filter_degree(&u_buf->edges[thread_work[tid].start],thread_work[tid].stop - thread_work[tid].start,u_buf->curr_ts);
               //                      continue;
               //                    }
#if SYM == 1
//                                    if(degree[src]< K - 1 || degree[dst] < K-1) continue; //algo->prefilter(src) || !algo->prefilter(dst)) continue;

               //                     if(filter_cache[src].valid() || filter_cache[dst].valid() ) continue;
               ign.clear();
               //                     if(src %K == dst %K || degree[dst] < K-1 || degree[src] < K - 1) continue;
               embedding.append(src);

               embedding.append(dst);

               if(!algo->filter(dst,&embedding, 1)) {
                  embedding.pop();
                  embedding.pop();
                  continue;
               }
               uint32_t d,ts;
               FOREACH_EDGE_TS(src, d,ts)
                  if(d != dst && ts <= embedding.max_ts()) {
                     if (ts == embedding.max_ts() && (d < src || d < dst)) {  ign.insert(d);}
                  }
               ENDFOR
                  FOREACH_EDGE_TS(dst, d, ts)
                  //            if(d == src) continue;
                  if (ts == embedding.max_ts() &&  d < src ) {ign.insert(d);}
               ENDFOR
                  ////          assert(set.no_edges_in_pattern != 0);
                  explore_middleout( &embedding, 2, tid,&ign);
#else

               std::unordered_set<uint32_t> neighbours;
               neighbours.clear();
               ignore_bit.reset();
               embedding.append(src);
               embedding.append(dst);

               uint32_t d, ts;
               FOREACH_EDGE_TS(src, d, ts)
                  if (d != dst && ts <= embedding.max_ts()) {
                     if (ts == embedding.max_ts() && (d < src)) {
                        ignore_bit.set(d);
                        continue;
                     }
                     neighbours.insert(d);
                  }
               ENDFOR
                  explore_middleout(&embedding, 2, tid, &neighbours, &ignore_bit);
#endif
               embedding.pop();
               embedding.pop();
            }
         }
end:
         auto end_t = std::chrono::high_resolution_clock::now();

         std::chrono::duration<double, std::milli> diff = end_t - start;
               printf("(%d) %.3f | ", tid, diff.count()/1000.);
         wait_b(&xsync_end);

         if (tid != 0) goto begin;
        else {printf("\n");}
      }
      void compute(void *c) {

         int tid = (long) c;
         assert(tid<no_threads);

begin:

         wait_b(&xsync_begin);
         uint64_t prev_upd = 0;
         auto start = std::chrono::high_resolution_clock::now();

         Embedding<uint32_t> embedding;
         while (curr_item <no_active) {
            get_work(tid, &thread_work[tid], no_active);

            if (thread_work[tid].start == thread_work[tid].stop) goto end;
            for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
               uint32_t src,dst;
               if(do_updates){
                  src = edges[thread_work[tid].start].src;
                  dst = edges[thread_work[tid].start].dst;
               }
               else {
                  src = edges[active[thread_work[tid].start]].src;
                  dst = edges[active[thread_work[tid].start]].dst;
               }
#if SYM== 1

                                   if(degree[src] < K- 1 || degree[dst] < K - 1)continue;// || src % K == dst % K) continue;
#endif
               if(dst < src ) continue;
#if SYM !=1

               std::unordered_set<uint32_t> neighbours = std::unordered_set<uint32_t>();
               uint32_t d;
               neighbours.clear();
               FOREACH_EDGE(src, d)
                  if(canonic_check_r1E(d, &embedding, step)  && d!= dst)
                     neighbours.insert(d);
               ENDFOR

#endif
                  embedding.append(src);
               embedding.append(dst);
               //          if (dst < src) continue;

               //                              if (!algo->prefilter(src) || !algo->prefilter(dst)) continue;


//               if(!algo->filter(dst,&embedding,1)) { embedding.pop(); embedding.pop(); continue;}
#if SYM== 1
               explore_v(&embedding, 2, tid);
#else
               explore_v(&embedding, 2, tid, &neighbours);
#endif
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

         if (tid != 0 &&!do_updates) goto begin;
      }


      void execute_app() {
         printf("No active %lu\n",no_active);
#if SYM == 1
         printf("Running in symmetric mode \n");
#else
         printf("Running in unsymmetric mode \n");
#endif



         if(!do_updates){

            for (int i = 0; i < no_threads - 1; i++) {
               threads[i] = new std::thread(&Engine<T,A>::compute, this, (void *) (i + 1));
            }

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
         //
         u_buf = new UpdateBuffer(batch_size , edges_full, NB_EDGES, NB_NODES ,initial_chunk,no_threads);
         //
         if(initial_chunk != 0 && do_updates){
            memset(degree,0, NB_NODES*sizeof(uint32_t));
            for (int i = 0; i < no_threads - 1; i++) {
               threads[i] = new std::thread(&UpdateBuffer::preload_edges_before_update, u_buf, edges_full, (i + 1), edges, (int)no_threads);
            }
            u_buf->preload_edges_before_update(edges_full, 0, edges,(int)no_threads);
            for (int i = 0; i < no_threads - 1; i++) {
               threads[i] = new std::thread(&Engine<T,A>::compute, this, (void *) (i + 1));
            }
            no_active = initial_chunk;
            curr_item = 0;
            compute(0);
            no_triangles = 0;
            for (int i = 0; i < no_threads; i++) {
               no_triangles += per_thread_data[i];
            }
            printf("Preloaded batch of %lu and found %lu\n",initial_chunk,no_triangles);

         }
         for (int i = 0; i < no_threads - 1; i++) {
            threads[i] = new std::thread(&Engine<T,A>::compute_middleout, this, (void *) (i + 1));
         }

         //               free (edges_full);
         uint64_t items_processed = 0;
         float total_time = 0;
         do {
            curr_item = 0;
            uint64_t cand = 0;
            curr_cliques = 0;

            //        u_buf->set_batch_size();
           auto start_time = std::chrono::high_resolution_clock::now();
            u_buf->update_graph_structure(edges,edges_full, w_id,no_workers);
            no_active = u_buf->get_no_updates();//u_buf->u_buf->curr_batch_end - u_buf->curr_batch_start;
            printf("\n[BATCH %u - %u ](%lu): ",u_buf->curr_batch_start,u_buf->curr_batch_end,no_active);

            //
            //                  compute_middleout(0);
            auto end_time = std::chrono::high_resolution_clock::now();

           std::chrono::duration<double, std::milli> diff = end_time - start_time;
           printf("Time to apply batch %.3f\n",  diff.count()/1000.);
            //

            ////                  if(items_processed != 0)
            ////                  total_time += diff.count()/1000.;
            //                  printf("Time to create filter %.3f\n",  diff.count()/1000.);
            //
            //                  phase = 1 - phase;
            curr_item = 0;

            start_time = std::chrono::high_resolution_clock::now();

            compute_middleout(0);
            end_time = std::chrono::high_resolution_clock::now();
            phase = 1- phase;
            diff = end_time - start_time;
            //                   if(items_processed != 0)
            total_time += diff.count()/1000.;
            printf("Time to  process batch %.3f ",  diff.count()/1000.);

            no_active = no_active_next;
            no_triangles = 0;
            for (int i = 0; i < no_threads; i++) {
               no_triangles += per_thread_data[i];
               per_thread_data[i] = 0;
            }
           size_t no_checks = 0;
           for(int i = 0; i < no_threads;i++){
             no_checks += per_thread_candidates[i];
             per_thread_candidates[i] =0;
           }
            items_processed += no_triangles;
            printf("Found %lu ; checks %lu \n",no_triangles,no_checks);

            if(e_cache_enabled) {
                for (size_t i = 0; i < no_threads; ++i) {
                    //fprintf(stderr, "Updating cache for thread %lu with %lu items...", i, per_thread_e_cache_buffer[i].size());
                    for (size_t j = 0; j < per_thread_e_cache_buffer[i].size(); ++j) {
                        uint32_t first = per_thread_e_cache_buffer[i][j][0], second = per_thread_e_cache_buffer[i][j][1], third = per_thread_e_cache_buffer[i][j][2];
                        sort3(first, second, third);
                        e_cache->insert(first, second, third, &per_thread_e_cache_buffer[i][j]);
                    }
                    per_thread_e_cache_buffer[i].clear();
                    //fprintf(stderr, " done!\n");
                }
                //fprintf(stderr, "Cache contains %lu entries!\n", e_cache->num_entries());
            }

         }while(u_buf->has_work()); //u_buf->curr_batch_end < u_buf->no_edges );//- initial_chunk);
         uint64_t no_checks =0 ;
         //      for (int i = 0; i < no_threads; i++) {
         //        no_checks += per_thread_candidates[i];
         ////        per_thread_candidates[i] = 0;
         //      }
         printf("Total discovered %lu in %.3f s\n",items_processed, total_time);
      }

   };

#endif
