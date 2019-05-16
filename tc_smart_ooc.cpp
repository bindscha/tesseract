
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
 *                   add n1 trangle_list[v1] - optional
 *                    no_triangle++
 *
 * Note this will not work if the adjacency list has the same neighbour more than once.
 *
 *  */
#include "parallel_ligra.hpp"
#include "algo_api.hpp"
//#include "radixSort_ligra.hpp"
#include "barrier.hpp"
//#include "adj_thread_work.hpp"
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

#define NB_NODES 4847571LU//39459925LU //4847571LU
#define CHUNK_SIZE 1
#define  no_threads 56
struct edge{
    uint32_t dst;
};

int phase = 0;
void init_adj_degree();
uint64_t NB_EDGES;
struct n_triangle{
    uint32_t curr_candidates = 0;
    // uint32_t* candidates;
};

uint32_t* candidates;
struct n_triangle* triangles; // [NB_NODES];

bool should_be_active(uint32_t v_id){ //Algo defined pruning function
  return degree[v_id] >= 2;
}
uint32_t degree[NB_NODES];
uint32_t degree_in[NB_NODES];
size_t* adj_offsets;
size_t* adj_offsets_in;

char* degree_file;
char* input_file;
uint32_t active[NB_NODES];
uint32_t active_next[NB_NODES];

bool* in_frontier;
uint64_t no_triangles = 0;
struct edge* edges;
uint32_t no_active, no_active_next;

uint32_t per_thread_data[no_threads];


x_barrier xsync_begin, xsync_end;
struct thread_work_t{
    uint32_t start;
    uint32_t stop;
};

thread_work_t thread_work[no_threads];

uint32_t curr_item = 0;
void get_work(int tid, thread_work_t* t_work, uint32_t max){
  uint32_t num = max / no_threads;
  uint32_t incr = CHUNK_SIZE;

  uint32_t idx = __sync_fetch_and_add(&curr_item, incr);

  if(idx >=max) {t_work->start = t_work->stop = max; return;}

  t_work->start = idx;

  t_work->stop = t_work->start + incr;
  if(t_work->stop >max )t_work->stop = max;


}

void* compute(void* c){

  int tid = (long)c;
  begin:

  wait_b(&xsync_begin);

  while(curr_item    < no_active) {
    get_work(tid, &thread_work[tid], no_active);
    switch (phase) {
      case 0: {

        for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
          uint32_t src = active[thread_work[tid].start];


          for(uint32_t n = 0; n < degree[src]; n++){
            uint32_t dst = edges[adj_offsets[src] + n].dst;

            if(dst > src) {//continue;
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

        for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
          int32_t src = active_next[thread_work[tid].start];

          for (uint32_t n = 0; n < degree[src]; n++) {
            uint32_t dst = edges[adj_offsets[src] + n].dst;
            if (dst <= src) continue;
            for (uint32_t idx = 0; idx < degree[dst]; idx++) {
              uint32_t n_dst = edges[adj_offsets[dst] + idx].dst;
              if (n_dst >= src) continue;
              for (uint32_t cand = 0; cand < triangles[src].curr_candidates; cand++) {
                if (n_dst == candidates[adj_offsets_in[src]+ cand])
                  per_thread_data[tid]++; // no_triangles++;
                /*
                 * There is no explicit output of triangles here, just counting
                 */
              }
            }
          }
        }
      }
        break;
    }

  }
  wait_b(&xsync_end);
  if(tid != 0 ) goto begin;
}


int main(int argc, char** argv)  {

  int cand_fd = open("/media/nvme/c_fd", O_RDWR |O_CREAT, 0700); //mmaped intermediary state (candidates)

  assert(cand_fd!=-1);

  triangles = (n_triangle*) calloc(NB_NODES , sizeof(n_triangle));

  in_frontier = (bool*) calloc(NB_NODES ,sizeof(bool));

  degree_file = argv[1];
  input_file = argv[2];
  memset(per_thread_data, 0, no_threads * sizeof(uint32_t));
  int fd = open(input_file, O_RDWR);
  if(fd == -1) {
    perror("Failed to open input file");
    exit(1);
  }
  struct stat sb;
  fstat(fd, &sb);


  NB_EDGES = (size_t) sb.st_size / sizeof(struct edge);
  printf("NB EDGES %lu\n", NB_EDGES);
  edges = (struct edge*) mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if(edges== MAP_FAILED) {
    perror("Failed mapping");
    exit(1);
  }
  init_adj_degree();

  uint64_t to_mmap = NB_EDGES*sizeof(uint32_t);
  if(fallocate(cand_fd, 0, 0, NB_EDGES * sizeof(uint32_t)) == -1){
    perror("Failed to allocate");
    exit(1);
  }
  candidates = (uint32_t*) mmap(NULL, NB_EDGES * sizeof(uint32_t),PROT_READ|PROT_WRITE, MAP_SHARED, cand_fd, 0);
  if(candidates == MAP_FAILED){
    perror("Failed to mmap");
    exit(1);
  }
  no_active = NB_NODES;
  no_active_next = 0;

  init_barrier(&xsync_begin, no_threads);
  init_barrier(&xsync_end, no_threads);

  /*
   * This is a bit of hack.Right now, we assume there can't be more candidates than the in-degree so we compute the in-degre of nodes.
   * In phase 0, more than 1 thread adds candidates for a node, but we don't need to sync on the entire array, just on the per node candiadte counter.
   * There is less chance for conflict and more parallelism this way.
   */
  memset(degree_in, 0, NB_NODES* sizeof(uint32_t));
  adj_offsets_in = (size_t*) calloc(NB_NODES, sizeof(*adj_offsets)); //(NULL, NB_NODES * sizeof(*adj_offsets), PROT_READ , MAP_PRIVATE, fd,0);

  parallel_for(uint32_t i = 0; i <NB_NODES; i++){
    for(size_t idx = 0; idx< degree[i]; idx++){
      __sync_fetch_and_add(&degree_in[edges[adj_offsets[i]+ idx].dst],1);
    }
  }

  adj_offsets_in[0] = 0;
  for(uint32_t i = 1; i <NB_NODES;i++){
    adj_offsets_in[i] = degree_in[i-1] + adj_offsets_in[i-1];
  }

/*
 * End of in_degree compute
 */

  pthread_t threads[no_threads-1];


  parallel_for(uint32_t i = 0; i < NB_NODES; i++){
    //Optional pruning here (nodes with a small degree can be pruned, but only if the graph is undirected
    active[i] = i;

  }


  for(int i = 0; i < no_threads-1;i++){
    pthread_create(&threads[i], NULL, compute, (void*)(i+1));
  }

  compute(0);


  printf("Done with phase 1\n");

  no_active = no_active_next;


  phase = 1 -phase;
  curr_item = 0;
  compute(0);

  for(int i = 0; i < no_threads; i++)
    no_triangles += per_thread_data[i];
  printf("Number of triangles %lu\n", no_triangles);

  for(int i =0; i < no_threads -1; i++){
    pthread_cancel(threads[i]);
  }

}


void init_adj_degree(){
  int fd = open(degree_file, O_RDONLY, O_NOATIME);
  if(fd == -1) {
    perror("File open failed");
    exit(1);

  }
  adj_offsets = (size_t*) mmap(NULL, NB_NODES * sizeof(*adj_offsets), PROT_READ , MAP_PRIVATE, fd,0);

  if(adj_offsets == MAP_FAILED)
    perror("Failed to mmap offsets");

  parallel_for(uint32_t i = 0; i < NB_NODES -1; i++){
    degree[i] = adj_offsets[i+1] - adj_offsets[i];
  }
  degree[NB_NODES - 1] = NB_EDGES - adj_offsets[NB_NODES - 1];
}
