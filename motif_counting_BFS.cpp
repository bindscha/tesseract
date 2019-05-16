
/*
 * void explore(uint32_t src, Pattern set, int step, int thread_id){
 *  count = 0
 *  for dst in neighbours(src) //Check if this node is connected to all the other nodes in the set
 *    if dst < src && in_set(set, dst, step) count++
 *    if count == step break
 *
 *  if (count != step) return // if not return
 *
 *  if (step < K )
 *    for dst in neigbours(src)
 *      if dst < src continue
 *      set[step] = src     //Add the source to the set and start exploring from this dst
 *      explore(dst, step, step + 1, tid)
 *
 *  if(step == K && count == step)
 *    set[step] = src
 *    add item in thread local vertices_in_pattern (set)
 *    incr count of cliques
 *  }
 *
 *
 * main (){
 *  parallel_for all src in nodes
 *    set = {src}
 *    for all dst in neighbours(src)
 *      if dst < src
 *         explore (dst, set, 1, tid)
 *
 *
 * }
 *
 *
 *
 *
 *  */
#include "common_include.hpp"
#include "barrier.hpp"
#include "algo_api.hpp"


#include "utils.hpp"
#include "graph.hpp"

#include "clique_vector.hpp"

#include "Pattern.hpp"
#include "motif_counting.hpp"
#include "thread_data.hpp"
size_t discarded = 0;
int max = K;
//#define K 3

#define NO_ITEM 78425611LU
/*
 * Algo data
 *
 */

Pattern<uint32_t, edge_full>* cliques_found; //this is mmaped to a file; TODO it could be done a bit better
Pattern<uint32_t, edge_full>* cliques_found_next; //this is mmaped to a file; TODO it could be done a bit better
size_t curr_cliques = 0;

int phase = 0;

int step = 2;


uint64_t no_triangles = 0;
//Pattern<uint32_t, edge_full> pattern_list;



void* compute(void* c){

  int tid = (long)c;
  begin:

  wait_b(&xsync_begin);

  uint64_t prev_upd = 0;
//  if(tid != 0 )goto end;
  while(curr_item    < no_active) {
    get_work(tid, &thread_work[tid], no_active);

    if(thread_work[tid].start == thread_work[tid].stop) goto end;


    for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
//      uint32_t p_id = active[thread_work[tid].start];
      Pattern<uint32_t, edge_full>* set = &cliques_found[thread_work[tid].start ];


      auto start = std::chrono::high_resolution_clock::now();

      uint32_t dst;
      std::set<uint32_t> neighbours;

      for(int i = 0; i < step; i++) {
        uint32_t v_id = set->vertices_in_pattern[i];

        FOREACH_EDGE(v_id,dst)
        neighbours.insert(dst);
        ENDFOR

      }
//      for(int i = 0; i <step; i++) {
        for (uint32_t n: neighbours) {
          if (!canonic_check_v(n, set, step)) continue;
          if (!exec_filter < uint32_t, Pattern < uint32_t, edge_full >> (&filter_motif, n, set, step + 1)) continue;
          bool should_expand = exec_expand < uint32_t, Pattern<uint32_t, edge_full>>
          (&expand, n, set, step);
          set->vertices_in_pattern[step] = n;
          if (step != K - 1) {
            add_item_in_buf_gen < Pattern < uint32_t, edge_full >> (cliques_found_next, *set, tid, &(no_active_next));
          }
          per_thread_data[tid]++;

        }
//      }
//      for(int i = 0; i < step; i++) {
//        uint32_t v_id = set->vertices_in_pattern[i];
//
//
//        FOREACH_EDGE(v_id, dst)
//        if(!canonic_check(dst, set, step,v_id )) {
//          continue;
//        }
//
//        bool is_cand = exec_filter<uint32_t, Pattern<uint32_t, edge_full>>(&filter_motif, v_id, set, step+1);
//        if(!is_cand) continue;
//
////        bool should_expand =  exec_expand<uint32_t, Pattern<uint32_t, edge_full>>(&expand, v_id, set, step);
//
//        set->vertices_in_pattern[step] = dst;
//        if(step != K -1)
//          add_item_in_buf_gen<Pattern<uint32_t, edge_full>>(cliques_found_next, *set, tid, &(no_active_next));
////        else
////          exec_process<Pattern<uint32_t, edge_full>>(&process,set, step);
//        per_thread_data[tid]++;
//
////          cliques_found[p_id].vertices_in_pattern[step] = dst;
//
////          printf("(%u) %u->%u {", step, v_id, dst);
////          for(int k = 0; k <step + 1; k++){
////            printf("%u,",set.vertices_in_pattern[k]);
////          }
////          printf("} \n");
//
//
//        ENDFOR
//      }

      auto end = std::chrono::high_resolution_clock::now();
//          std::chrono::duration<double> diff = end - start;
      std::chrono::duration<double, std::milli> diff = end - start;

//    if(per_thread_data[tid] - prev_upd != 0) printf("%lu %u %.3f\n", per_thread_data[tid] - prev_upd,  src,diff.count());//src<<" "<< diff.count() << "\n";
      prev_upd = per_thread_data[tid];
    }
  }
  end:
  if(thread_bufs_gen[tid].curr != 0 && step != K -1) {
    flush_buf_gen(cliques_found_next, tid, &no_active_next);
  }
//  printf("Done %d\n",tid);
  wait_b(&xsync_end);

  if(tid != 0 ) goto begin;
}





int main(int argc, char** argv)  {


  auto start = std::chrono::high_resolution_clock::now();
  cliques_found = (Pattern<uint32_t, edge_full>*) malloc(NO_ITEM * sizeof(Pattern<uint32_t, edge_full>));
  cliques_found_next = (Pattern<uint32_t, edge_full>*) malloc(NO_ITEM *sizeof(Pattern<uint32_t, edge_full>));
  DIRECTION = FWD;
  degree_file = argv[1];
  input_file = argv[2];
  pthread_t threads[no_threads-1];

  init();
  init_thread_d(clique_fd);
  no_active = 0;
  for(uint32_t i = 0; i < NB_NODES; i++){
    cliques_found[no_active].alloc_vertices(max);// = Pattern<uint32_t , edge_full>(max);
    cliques_found_next[no_active].alloc_vertices(max);// = Pattern<uint32_t , edge_full>(max);
    cliques_found[no_active].vertices_in_pattern[0] = i;
    active[no_active++] = i;


  }

  for(size_t i = NB_NODES; i <NO_ITEM; i++){
    cliques_found[i].alloc_vertices(max);// = Pattern<uint32_t , edge_full>(max);
    cliques_found_next[i].alloc_vertices(max);// = Pattern<uint32_t , edge_full>(max);
  }
  thread_bufs_gen = (thread_buf_t_<Pattern<uint32_t, edge_full>>*) malloc(no_threads * sizeof(thread_buf_t_<Pattern<uint32_t, edge_full>>));
  for(int i = 0; i < no_threads;i++){
    thread_bufs_gen[i].curr =0 ;
    for(int j = 0; j < 2048; j++)
      thread_bufs_gen[i].buffer[i].alloc_vertices(max);
  }
  printf("no _active = %u\n",no_active);
  for(int i = 0; i < no_threads-1;i++){
    pthread_create(&threads[i], NULL, compute, (void*)(i+1));
  }
  step = 1;
  while(step < K) {
    curr_item = 0;
    uint64_t cand = 0;
    curr_cliques = 0;

    compute(0);

//    for(size_t i = 0; i <no_active_next;i++){
//      active[i] = i;
//    }
    printf("Done \n");
    no_active = no_active_next;
    printf("No active next %lu\n", no_active_next);
    no_active_next = 0;
    Pattern<uint32_t, edge_full>* tmp = cliques_found;
    cliques_found = cliques_found_next;
    cliques_found_next = tmp;
    step++;
    curr_item = 0;
   no_triangles = 0;

    for (int i = 0; i < no_threads; i++) {
      no_triangles += per_thread_data[i];
      per_thread_data[i] = 0;
    }


    printf("Number active %lu, motifs %lu\n", no_active, no_triangles);

  }


  for(int i =0; i < no_threads -1; i++){
    pthread_cancel(threads[i]);
  }
//  printf("All done\n");
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << "Finish triangle counting. Running time : " << diff.count() << " s\n";

//  fsync(clique_fd);
}









