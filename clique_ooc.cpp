
/*
 * Phase 1
 * For src in active_nodes
 *    for n in neighbours(src)
 *      if (n > src) - optional additional pruning
 *          add n to candidates of src
 *          if n not marked active for next iteration
 *              mark n as active
 *              add to active_next queue
 *
 *  swap(active_nodes, active_next)
 *  Phase 2
 * for src in active_nodes
 *    for n in neighbours(src)
 *       if n > src  - we only explore the vertices with a higher degree; optional additional pruning here
 *          for n1 in neighbours of n
 *            if n1 < src - optional additional pruning here
 *               for v1 in candidates of src
 *                 if n1 == v1
 *                    setbit (trianblebit(src))
 *                   add n1 trangle_list[v1]
 *                    no_triangle++
 *
 * Note this will not work if the adjacency list has the same neighbour more than once.
 * K = 4 here - this is the N in N-cliques
 * step = 3 - currently discovered clique size - triangles
 *
 * while(step < K){
 *  foreach subgraph discovered in step -1 do:
 *    count = 0
 *    copy = 0
 *    src = get_last_item(subgraph)
 *    for (n in Neighbours of src)
 *      if( n  < src) cont; - we only expand towards vertices with a higher degree; optional pruning condition here
 *      for n1 in neighbours of n
 *          if (n1 < src)
 *            if(n1 in subgraph) count++
 *      if count == step // all the members of the subgraph are also neighbours of n
 *
 *            if(cpy)       //we need this because we can have more than one clique with the same subgraph
 *              create subgraph' = subgraph[0-step] + n
 *            else {
 *              subgraph.append (n)
 *              cpy = 1
 *           }
 *           if(step == K) no_cliques++
 *           else
 *              mark subgraph(or subgraph') to be explored in the next iteration
 *
 *
 *      else
 *         delete triangle // this has not been done, it should be, it saves memory
 *
 *
 *
 *
 *  }
 *
 *
 *
 * }
 *
 * For Cliques we build on the above, but add outputting of the triangles in phase 1. The compute has a phase 2 which should be repeated as long as
 * still have cliques to discover
 *  */

#include "parallel_ligra.hpp"
#include "barrier.hpp"
#include "algo_api.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include<pthread.h>
#include<algorithm>
#include<stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <omp.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <sys/sysinfo.h>
#include <sys/syscall.h>
#include <sched.h>
#include <vector>
#include <forward_list>
#include <iostream>
#include <functional>

#include "graph.hpp"
#include "clique_vector.hpp"

#include "cliques_ooc.hpp"
#include "thread_data.hpp"
using namespace std::placeholders;

//
//bool filter(clique_vector set, uint32_t v_id, uint32_t step){
//  clique_vector v = (clique_vector)set;
//
//  for(uint32_t i = 0; i < step; i++){
//    if(v_id == v.buffer[i] ) return true;
//  }
//  return false;
//}

size_t discarded = 0;


/*
 * Algo data
 *
 */

bool filter_bfs(uint32_t src, clique_vector set, uint32_t step){
  uint32_t count = 0;
  for(uint32_t i = 0; i <degree[src];i++){
    uint32_t dst = edges[adj_offsets[src] + i].dst;

//should have a wrapper to loop over edges in one direction and hide the second condition from the end user
    if(should_be_active(dst) && dst < src &&   is_in_set(set, dst, step)) count++;

//            propagation_cond(dst, src , BACK) &&
    if(count == step -1) break;
  }
  return count == step - 1;
}
clique_vector* cliques_found; //this is mmaped to a file; TODO it could be done a bit better
size_t curr_cliques = 0;

int phase = 0;

uint32_t step = 2;
struct n_triangle{
    uint32_t curr_candidates = 0;
};

uint32_t* candidates;
struct n_triangle* triangles;
uint64_t no_triangles = 0;



/*
 * functions
 */

void init(int cand_fd, int edge_fd);

bool in_set(uint32_t* set, uint32_t v_id, uint32_t step){
  for(uint32_t i = 0; i < step; i++){
    if(v_id == set[i]) return true;
  }
  return false;
}

void* compute(void* c){

  int tid = (long)c;
  begin:

  wait_b(&xsync_begin);

  while(curr_item    < no_active) {
    get_work(tid, &thread_work[tid], no_active);
    if(thread_work[tid].start == thread_work[tid].stop) goto end;
//    assert(curr_item <= no_active);
    switch (phase) {
      case 0: {
//            printf("Phase %d\n",phase);
        for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
          uint32_t src = active[thread_work[tid].start];


          for(uint32_t n = 0; n < degree[src]; n++){
            assert(degree[src] < NB_NODES);
            uint32_t dst = edges[adj_offsets[src] + n].dst;
//            int reverse = 0;
//            volatile uint32_t last_pos = 0;
//            for(uint32_t idx = 0; idx < degree[dst]; idx++){
//              uint32_t tmp = edges[adj_offsets[dst] + idx].dst;
//              if(tmp == src) {
//                reverse++;
//
//
//                if(reverse > 1){
//                  printf("reverse is wrong %d %lu (%lu)\n",reverse, last_pos, idx);
//                  exit(1);
//                }
//                last_pos = idx;
//              }
//
//            }
//
//
//            if(reverse == 0){
//              printf("error, not undirected\n");
//              exit(1);
//            }
//            assert(reverse <= 1);

            if(dst > src && should_be_active(dst)) {//continue;
              if (__sync_bool_compare_and_swap(&in_frontier[dst], 0, 1)) {
                active_next[__sync_fetch_and_add(&no_active_next, 1)] = dst;

              }

              uint32_t idx = __sync_fetch_and_add(&triangles[dst].curr_candidates, 1);
              candidates[adj_offsets_in[dst] + idx] = src;
              assert(triangles[dst].curr_candidates <= degree_in[dst]);

            }
          }
        }
        break;
      }
      case 1: {
//            printf("Phase %d\n",phase);
        for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
          uint32_t src = active_next[thread_work[tid].start];

          for (uint32_t n = 0; n < degree[src]; n++) {
            uint32_t dst = edges[adj_offsets[src] + n].dst;
            if ( !(propagation_cond(dst,src, DIRECTION)) || !should_be_active(dst)) { continue;}
            for (uint32_t idx = 0; idx < degree[dst]; idx++) {
              uint32_t n_dst = edges[adj_offsets[dst] + idx].dst;
              bool copy = 0;
              if (!propagation_cond(n_dst, src, BACK) || !should_be_active(n_dst)) {continue;}
              uint32_t added = 0;
              for (uint32_t cand = 0; cand < triangles[src].curr_candidates; cand++) {

                if (n_dst == candidates[adj_offsets_in[src]+ cand]) {
                  per_thread_data[tid]++;

                  add_item_in_buf(cliques_found, clique_vector(src,n_dst,dst),tid, &curr_cliques);
                  break;

                }

              }

            }
          }
        }
        if(thread_bufs[tid].curr != 0) {
          flush_buf(cliques_found, tid, &curr_cliques);
        }
        break;
      }

      case 2: {

        for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
           uint32_t src = cliques_found[thread_work[tid].start].buffer[step-1];//.at(step-1);

          if(src == -1U){cliques_found[thread_work[tid].start].buffer[step] = -1U; continue;}
            uint32_t count = 0;
            bool copy = 0;
//          printf("\n %u, %u %u -> ", cliques_found[thread_work[tid].start][0], cliques_found[thread_work[tid].start][1],src );
          for(uint32_t i = 0; i < degree[src];i++) {
            count = 0;
              uint32_t dst = edges[adj_offsets[src] + i].dst;
//            printf("Exploring from %u to %u: ",src, dst);

            bool is_cand;
              if(propagation_cond(dst,src, DIRECTION) && should_be_active(dst)) {
                  is_cand = exec_filter<uint32_t, clique_vector>(&filter_bfs, dst, cliques_found[thread_work[tid].start], step );
                  for(uint32_t j = 0; j < degree[dst]; j++) {
                    uint32_t n_id = edges[adj_offsets[dst] + j].dst;
//                    printf("%u ",n_id);
//                    auto f1 = std::bind(exec_filter<clique_vector>(filter, cliques_found[thread_work[tid].start], n_id, step));//, );
                    if (n_id < src ){// && exec_filter<clique_vector>(&filter, cliques_found[thread_work[tid].start], n_id, step)) {
//                        count++;
                      for(uint32_t k = 0; k < step; k++){
                        uint32_t item = cliques_found[thread_work[tid].start].buffer[k];

//                      for (uint32_t &item: cliques_found[thread_work[tid].start]) {
                        if (item == n_id) {
//                          printf("* ");
                          count++; break;
                        }
                      }
                    }
                    if (count == step - 1) break;
                  }

                if (count == step - 1 && step != K - 1) {
                  per_thread_data[tid]++;
                  if(copy && step < (K-1)) {
                    add_item_in_buf(cliques_found, clique_vector(cliques_found[thread_work[tid].start], dst ), tid, &curr_cliques);
                  }
                  else {
                    cliques_found[thread_work[tid].start].buffer[step] = dst;//.push_back(dst);// = dst; //.push_back(dst);
                    copy = 1;
                  }
                 }
            }
          }
          if(!copy){
//            discarded++;//
//            __sync_fetch_and_add(&discarded, 1);
            cliques_found[thread_work[tid].start].buffer[step]= -1U;//.push_back(100); //this should be delete
          }
        }
        if(thread_bufs[tid].curr != 0 && step != K -1){
          flush_buf(cliques_found, tid, &curr_cliques);
        }
        break;
      }
    }
  }
  end:
    wait_b(&xsync_end);
    if(tid != 0 ) goto begin;
}

void init_bfs(){

  //Open edge file
  int fd = open(input_file, O_RDWR);
  if(fd == -1) {
    perror("Failed to open input file");
    exit(1);
  }
  struct stat sb;
  fstat(fd, &sb);
  bfs = 1;

  NB_EDGES = (size_t) sb.st_size / sizeof(struct edge);
  printf("NB EDGES %lu\n", NB_EDGES);


  edges = (struct edge*) mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if(edges== MAP_FAILED) {
    perror("Failed mapping");
    exit(1);
  }
  init_adj_degree();

  uint64_t to_sync = NB_EDGES * sizeof(edge);
  if(to_sync % 4096 != 0){
    to_sync = to_sync + 4096 - to_sync %4096 ;
  }

  assert(madvise(edges, to_sync, MADV_RANDOM) != -1);
  //Open candidate out file
  int cand_fd =  open("/media/nvme/c_fd", O_RDWR |O_CREAT, 0700);
  int clique_fd =  open("/media/nvme/clique_fd", O_RDWR |O_CREAT , 0700);
//  int clique_fd2 =  open("/media/nvme/clique_fd", O_WRONLY |O_CREAT |O_APPEND, 0700);
  assert(cand_fd!=-1);

  uint64_t to_mmap = NB_EDGES*sizeof(uint32_t);
  assert(fallocate(cand_fd, 0, 0, NB_EDGES * sizeof(uint32_t)) != -1);

  candidates = (uint32_t*) mmap(NULL, NB_EDGES * sizeof(uint32_t),PROT_READ|PROT_WRITE, MAP_SHARED, cand_fd, 0);
  assert(candidates != MAP_FAILED);

  assert(fallocate(clique_fd, 0, 0, sizeof(uint32_t) * SIZE *K) != -1);
//  cliques_found = (clique_vector*)malloc(NB_EDGES * 130LU * 4LU);//sizeof(uint32_t) );//
  cliques_found = (clique_vector*)mmap(NULL, sizeof(uint32_t) * SIZE *K,PROT_READ|PROT_WRITE, MAP_SHARED, clique_fd, 0);
  assert(cliques_found != MAP_FAILED);
//  memset(cliques_found, 0, NB_EDGES *sizeof(uint32_t) * 130);

  to_sync =  sizeof(uint32_t) * SIZE *K;
  if(to_sync % 4096 != 0){
    to_sync = to_sync + 4096 - to_sync %4096 ;
  }

  assert(madvise(cliques_found, to_sync, MADV_SEQUENTIAL) != -1);
  //compute in degree
  memset(degree_in, 0, NB_NODES* sizeof(uint32_t));
  adj_offsets_in = (size_t*) calloc(NB_NODES, sizeof(*adj_offsets));
  parallel_for(uint32_t i = 0; i <NB_NODES; i++){
    for(size_t idx = 0; idx< degree[i]; idx++){
      __sync_fetch_and_add(&degree_in[edges[adj_offsets[i]+ idx].dst],1);
    }
  }
  adj_offsets_in[0] = 0;
  for(uint32_t i = 1; i <NB_NODES;i++){
    adj_offsets_in[i] = degree_in[i-1] + adj_offsets_in[i-1];
  }


  no_active = NB_NODES;
  no_active_next = 0;
  init_barrier(&xsync_begin, no_threads);
  init_barrier(&xsync_end, no_threads);


  triangles = (n_triangle*) calloc(NB_NODES , sizeof(n_triangle));
  in_frontier = (bool*) calloc(NB_NODES ,sizeof(bool));
  memset(per_thread_data, 0, no_threads * sizeof(uint32_t));


  thread_bufs = (thread_buf_t*) calloc(no_threads , sizeof(thread_buf_t));
  printf("Done with init\n");


  /*
   * buf data to realloc
   */
  file_fd = clique_fd;
  buf_size =  K * sizeof(uint32_t) * SIZE;
}




int main(int argc, char** argv)  {

  degree_file = argv[1];
  input_file = argv[2];
  pthread_t threads[no_threads-1];

  init_bfs();

no_active = 0;
  for(uint32_t i = 0; i < NB_NODES; i++){
    if(should_be_active[i])
    active[no_active++] = i;
  }
  for(int i = 0; i < no_threads-1;i++){
    pthread_create(&threads[i], NULL, compute, (void*)(i+1));
  }

  compute(0);

  printf("Done with phase 1\n");
  no_active = no_active_next;
  phase = 1 -phase;
  curr_item = 0;
  uint64_t cand = 0;
  curr_cliques = 0;

//  for(uint32_t i = 0; i <NB_NODES; i++){
//
//     cand += triangles[i].curr_candidates;
//
//  }
  printf("No candidates %lu\n",cand);
  compute(0);

  for(int i = 0; i < no_threads; i++) {
    no_triangles += per_thread_data[i];
    per_thread_data[i] = 0;
  }
  printf("Number of triangles %lu, %u discarded\n", no_triangles, discarded);
  phase = 2;
  step = 3;
  no_active = no_triangles;
  discarded = 0;
  printf("Number active %lu\n", no_active);
  uint64_t to_sync = K *sizeof(uint32_t) * SIZE;
  if(to_sync % 4096 != 0){
    to_sync = to_sync + 4096 - to_sync %4096 ;
  }
  assert(madvise(cliques_found, to_sync, MADV_SEQUENTIAL) != -1);

  while (step < K){
    curr_item = 0;
    compute(0);
    printf("Found %lu cliques of size %d\n", curr_cliques, step);
//    no_active = curr_cliques;
//    curr_cliques = 0;
    no_active = curr_cliques;
    step++;
    no_triangles = 0;
    for(int i = 0; i < no_threads; i++) {
      no_triangles += per_thread_data[i];
      per_thread_data[i] = 0;
    }
    printf("Number of cliques %lu; %lu discarded\n", no_triangles, discarded    );
  }


//  assert(msync(cliques_found, to_sync, MS_SYNC) != -1);
  for(int i =0; i < no_threads -1; i++){
    pthread_cancel(threads[i]);
  }

}





