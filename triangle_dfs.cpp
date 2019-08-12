
/*
 * void explore(uint32_t src, clique_vector set, int step, int thread_id){
 *  count = 0
 *  if(step == K)
 *  for dst in neighbours(src) //Check if this node is connected to all the other nodes in the set
 *    if dst < src  && dst == set[0]
 *      count++
 *      break
 *
 *
 *  if (!count) return // if not return
 *
 *  if (step < K )
 *    for dst in neigbours(src)
 *      if dst < src continue
 *      set[step] = src     //Add the source to the set and start exploring from this dst
 *      explore(dst, step, step + 1, tid)
 *
 *  if(step == K && count == step)
 *    set[step] = src
 *    add item in thread local buffer (set)
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
//#include "common_include.hpp"
//#include "barrier.hpp"
//#include "algo_api.hpp"
//
#include "engine.hpp"
#include "clique_vector.hpp"
//#include "utils.hpp"
#include "graph.hpp"
//
//
#include "thread_data.hpp"
//
//#include "triangle_c.hpp"


size_t discarded = 0;


/*
 * Algo data
 *
 */

clique_vector* cliques_found; //this is mmaped to a file; TODO it could be done a bit better
//size_t curr_cliques = 0;

//int phase = 0;
//
//int step = 2;
//
//uint64_t no_triangles = 0;

/*
 * functions
 */

void init(int cand_fd, int edge_fd);

void explore(uint32_t src, clique_vector set, uint32_t step, int tid){
  int count = 0;

  bool is_cand = exec_filter<uint32_t,clique_vector>(&triangle_filter, src, set, step);
  if(!is_cand) return;
  set.buffer[step] = src;
  if(step < K - 1){
    uint32_t dst;
    FOREACH_EDGE_FWD(src, dst) //If graph is directed we loop through all the edges then, but to avoid duplicates we need to make sure that hte graph does not have reverse edges
      explore(dst,set, step+1,tid);
    ENDFOR
  }

  if(step == K - 1 && is_cand){
    add_item_in_buf(cliques_found, set ,tid, &curr_cliques);
    per_thread_data[tid]++;
  }
}

void* compute(void* c){

  int tid = (long)c;
  begin:

  wait_b(&xsync_begin);

  while(curr_item    < no_active) {
    get_work(tid, &thread_work[tid], no_active);

    if(thread_work[tid].start == thread_work[tid].stop) goto end;


    for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
      uint32_t src = active[thread_work[tid].start];
      uint32_t dst;
      FOREACH_EDGE_FWD(src, dst)
            explore(dst, clique_vector(src, -1U, -1U), 1, tid);
      }

    }
  }
  end:
  if(thread_bufs[tid].curr != 0) {
    flush_buf(cliques_found, tid, &curr_cliques);
  }
//  printf("Done %d\n",tid);
  wait_b(&xsync_end);

  if(tid != 0 ) goto begin;
}






int main(int argc, char** argv)  {
  DIRECTION = FWD;
  degree_file = argv[1];
  input_file = argv[2];
  pthread_t threads[no_threads-1];

  init();
  init_thread_d(clique_fd);
  no_active = 0;
  for(uint32_t i = 0; i < NB_NODES; i++){

      active[no_active++] = i;

  }
  printf("no _active = %u\n",no_active);
  for(int i = 0; i < no_threads-1;i++){
    pthread_create(&threads[i], NULL, compute, (void*)(i+1));
  }
  curr_item = 0;
  uint64_t cand = 0;
  curr_cliques = 0;
  compute(0);

  printf("Done with compute\n");
  no_active = no_active_next;
  phase = 1 -phase;
  curr_item = 0;


  for(int i = 0; i < no_threads; i++) {
    no_triangles += per_thread_data[i];
    per_thread_data[i] = 0;
  }

  printf("Number of triangles %lu, %u discarded\n", no_triangles, discarded);

  printf("Number active %lu, curr_cliques %lu\n", no_active,curr_cliques);



  uint64_t to_sync = NB_EDGES * sizeof(uint32_t)*SIZE;
  if(to_sync % 4096 != 0){
    to_sync = to_sync + 4096 - to_sync %4096 ;
  }
//  assert(msync(cliques_found, to_sync, MS_SYNC) != -1);
  for(int i =0; i < no_threads -1; i++){
    pthread_cancel(threads[i]);
  }
  printf("All done\n");
//  fsync(clique_fd);
}









