
#include "parallel_ligra.hpp"
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

#define NB_NODES 39459925LU //4847571LU
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

uint32_t** candidates;
struct n_triangle* triangles; // [NB_NODES];

uint32_t degree[NB_NODES];
size_t* adj_offsets;

char* degree_file;
char* input_file;
uint32_t active[NB_NODES];
uint32_t active_next[NB_NODES];

bool* in_frontier;
uint64_t no_triangles = 0;
struct edge* edges;
uint32_t no_active, no_active_next;

uint32_t per_thread_data[no_threads];

pthread_spinlock_t* locks;
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
   if(CHUNK_SIZE + curr_item  >max)
      incr = max - curr_item;
   uint32_t idx = __sync_fetch_and_add(&curr_item, incr);
   if(idx >max) t_work->start = t_work->stop = max;

   t_work->start = idx;

   t_work->stop = t_work->start + incr;
   if(t_work->stop >max )t_work->stop = max;
//   if(tid == no_threads - 1) t_work->stop += max % no_threads;


}

void* compute(void* c){

   int tid = (long)c;
  begin:

   wait_b(&xsync_begin);

   while(curr_item    < no_active) {
      get_work(tid, &thread_work[tid], no_active);
      switch (phase) {
         case 0: {
//            printf("Phase %d\n",phase);
            for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
               uint32_t src = active[thread_work[tid].start];

               for (uint32_t n = 0; n < degree[src]; n++) {
                  assert(degree[src] < NB_NODES);
                  uint32_t dst = edges[adj_offsets[src] + n].dst;

                  if (dst <= src) continue;
                  if (__sync_bool_compare_and_swap(&in_frontier[dst], 0, 1)) { //, 0, 1){ // to be atomic

                     uint32_t next_idx = __sync_fetch_and_add(&no_active_next, 1);
                     active_next[next_idx] = dst; //to go to thread private buffer

                  }

                  pthread_spin_lock(&locks[dst]);
                  candidates[dst][triangles[dst].curr_candidates++] = src;
                  if (triangles[dst].curr_candidates % 4LU == 0) {
                     candidates[dst] = (uint32_t *) realloc(candidates[dst],
                                                            (triangles[dst].curr_candidates + 4LU) * sizeof(uint32_t));
                     if (!candidates[dst]) {
                        free(candidates[dst]);
                        exit(1);
                     }
                  }
                  pthread_spin_unlock(&locks[dst]);

               }
            }
            break;
         }
         case 1: {
//            printf("Phase %d\n",phase);
            for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
               int32_t src = active_next[thread_work[tid].start];

               for (uint32_t n = 0; n < degree[src]; n++) {
                  uint32_t dst = edges[adj_offsets[src] + n].dst;
                  if (dst <= src) continue;
                  for (uint32_t idx = 0; idx < degree[dst]; idx++) {
                     uint32_t n_dst = edges[adj_offsets[dst] + idx].dst;
                     if (n_dst >= src) continue;
                     for (uint32_t cand = 0; cand < triangles[src].curr_candidates; cand++) {
                        if (n_dst == candidates[src][cand])
                           per_thread_data[tid]++; // no_triangles++;
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

void old_main(int argc, char** argv){
   triangles = (n_triangle*) calloc(NB_NODES , sizeof(n_triangle));
   candidates = (uint32_t**) malloc(NB_NODES * sizeof(uint32_t*));
   in_frontier = (bool*) calloc(NB_NODES ,sizeof(bool));
   for(uint32_t i = 0; i < NB_NODES ; i++){
      candidates[i] = (uint32_t*) malloc(4 * sizeof(uint32_t));
   }
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

   uint64_t no_active = NB_NODES;
   uint64_t no_active_next = 0;

//   pthread_spinlock_t* locks;
//   locks = (pthread_spinlock_t*) malloc(NB_NODES* sizeof(pthread_spinlock_t));
   parallel_for(uint32_t i = 0; i < NB_NODES; i++){
      active[i] = i;
//      pthread_spin_init(&locks[i], PTHREAD_PROCESS_PRIVATE);
   }


//   parallel_
   for(uint32_t i = 0;  i < no_active;i++){
      uint32_t src = active[i];

      for(uint32_t n = 0; n < degree[src]; n++){
         assert(degree[src] < NB_NODES);
         uint32_t dst = edges[adj_offsets[src] + n].dst;

         if(dst <= src) continue;
//         if(__sync_bool_compare_and_swap(&in_frontier[dst] , 0,1)){ //, 0, 1){ // to be atomic
         if(in_frontier[dst] == 0){
            active_next[no_active_next++] = dst; //__sync_fetch_and_add(&no_active_next,1)] = dst; //to go to thread private buffer
            in_frontier[dst]  = 1;
         }
         //to be atomic

//         pthread_spin_lock(&locks[dst]);
         candidates[dst][triangles[dst].curr_candidates++] = src;
         if(triangles[dst].curr_candidates % 4LU == 0){
            candidates[dst]  = (uint32_t*) realloc(candidates[dst], (triangles[dst].curr_candidates + 4LU) * sizeof(uint32_t));
            if(!candidates[dst]) {
               free (candidates[dst]) ; exit(1);
            }
         }
//         pthread_spin_unlock(&locks[dst]);

      }    }
   printf("Done with phase 1\n");
   /*
    * for v in active_set
    *    for n in neighbours of v
    *       if n > v
    *          for n1 in neighbours of n
    *            if n1 < v
    *               for v1 in candidates of v
    *                 if n1 == v1
    *                    setbit (trianblebit(v))
    *                   add n1 tlist[v1]
    *                    no_triangle++
    *
    *
    *
    *  */


   parallel_for(uint32_t i = 0 ; i < no_active_next;i++){
      int tid = get_threadid();
      uint32_t src = active_next[i];

      for(uint32_t n = 0;  n < degree[src]; n++){
         uint32_t dst = edges[adj_offsets[src] + n].dst;
         if(dst <= src) continue;
         for(uint32_t idx = 0; idx < degree[dst]; idx++){
            uint32_t n_dst = edges[adj_offsets[dst] + idx].dst;
            if (n_dst >= src) continue;
            for(uint32_t cand = 0; cand < triangles[src].curr_candidates; cand++){
               if(n_dst == candidates[src][cand])
                  per_thread_data[tid]++; // no_triangles++;
            }
         }

      }

   }
   for(int i = 0; i < no_threads; i++)
      no_triangles += per_thread_data[i];
   printf("Number of triangles %lu\n", no_triangles);

}
int main(int argc, char** argv)  {
   triangles = (n_triangle*) calloc(NB_NODES , sizeof(n_triangle));
   candidates = (uint32_t**) malloc(NB_NODES * sizeof(uint32_t*));
   in_frontier = (bool*) calloc(NB_NODES ,sizeof(bool));
   for(uint32_t i = 0; i < NB_NODES ; i++){
      candidates[i] = (uint32_t*) malloc(4 * sizeof(uint32_t));
   }
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

   no_active = NB_NODES;
   no_active_next = 0;

   init_barrier(&xsync_begin, no_threads);
   init_barrier(&xsync_end, no_threads);


   locks = (pthread_spinlock_t*) malloc(NB_NODES* sizeof(pthread_spinlock_t));

   pthread_t threads[no_threads-1];


   parallel_for(uint32_t i = 0; i < NB_NODES; i++){
      active[i] = i;
      pthread_spin_init(&locks[i], PTHREAD_PROCESS_PRIVATE);
   }


   for(int i = 0; i < no_threads-1;i++){
      pthread_create(&threads[i], NULL, compute, (void*)(i+1));
   }

   compute(0);


   printf("Done with phase 1\n");
   /*
    * for v in active_set
    *    for n in neighbours of v
    *       if n > v
    *          for n1 in neighbours of n
    *            if n1 < v
    *               for v1 in candidates of v
    *                 if n1 == v1
    *                    setbit (trianblebit(v))
    *                   add n1 tlist[v1]
    *                    no_triangle++
    *
    *
    *
    *  */
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
