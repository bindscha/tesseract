#ifndef __THREAD_DATA_H__
#define __THREAD_DATA_H__

#include "Pattern.hpp"
/*
 * Thread sync data and thread work queues
 *
 */
size_t no_threads = 56;
#define CHUNK_SIZE 4//1024
int out = 0;
uint32_t curr_item = 0;

int file_fd;
x_barrier xsync_begin, xsync_end;
pthread_spinlock_t* locks;
int _realloc = 0;
struct thread_work_t{
    uint32_t start;
    uint32_t stop;
};
size_t *per_thread_data;
thread_work_t *thread_work;
/*
 *
 * Thread write buffers
 */
typedef int clique_vector; //TODOREMOVE
struct thread_buf_t{
    int curr;
    clique_vector buffer[2048];

};


bool bfs = 0;
thread_buf_t* thread_bufs;

template<typename T>
struct thread_buf_t_{
    int curr;
    T buffer[2048];

};


//extern thread_buf_t_* thread_bufs_gen;
thread_buf_t_<Pattern<uint32_t, edge_full> >* thread_bufs_gen;

void add_item_in_buf(clique_vector* cliques_found, clique_vector item, int tid, uint64_t* curr_cliques);
void flush_buf(clique_vector* cliques_found,int tid, uint64_t* curr_cliques);

template<typename T>
void add_item_in_buf_gen(T* cliques_found, T item, int tid, uint64_t* curr_cliques);

template<typename T>
void flush_buf_gen(T* cliques_found,int tid, uint64_t* curr_cliques);

void get_work(int tid, thread_work_t* t_work, uint32_t max);

void add_item_in_buf(clique_vector* cliques_found,clique_vector item, int tid, uint64_t* curr_cliques){
  thread_bufs[tid].buffer[thread_bufs[tid].curr] = item;


  thread_bufs[tid].curr++;
  if(thread_bufs[tid].curr == 2048)
    flush_buf( cliques_found, tid, curr_cliques);
}
template<typename T>
void add_item_in_buf_gen(T* cliques_found,T item, int tid, uint64_t* curr_cliques){
//  if(thread_bufs_gen[tid].buffer[thread_bufs_gen[tid].curr].max_size == 0){
//    thread_bufs_gen[tid].buffer[thread_bufs_gen[tid].curr].alloc_vertices(K);
//  }
  thread_bufs_gen[tid].buffer[thread_bufs_gen[tid].curr] = item;


  thread_bufs_gen[tid].curr++;
  if(thread_bufs_gen[tid].curr == 2048)
    flush_buf_gen( cliques_found, tid, curr_cliques);
}


template<typename T>
void flush_buf_gen(T* cliques_found, int tid, uint64_t* curr_cliques){
  uint64_t idx_to_add = __sync_fetch_and_add(curr_cliques, thread_bufs_gen[tid].curr);


//    size_t b_written = write(file_fd, thread_bufs_gen[tid].buffer, thread_bufs_gen[tid].curr * sizeof(T));//(sizeof(uint32_t) * K));
//    if (b_written == -1) {
//      perror("Failed to write\n");
//    }

//  memcpy(&cliques_found[idx_to_add], thread_bufs_gen[tid].buffer, thread_bufs_gen[tid].curr );
  for(size_t i = 0; i < thread_bufs_gen[tid].curr  ; i++){
    cliques_found[idx_to_add + i] = thread_bufs_gen[tid].buffer[i];
  }
  thread_bufs_gen[tid].curr = 0;
}

uint64_t buf_size = 0;

void init_thread_d(int c_fd){
  init_barrier(&xsync_begin, no_threads);
  init_barrier(&xsync_end, no_threads);

  per_thread_data = (size_t *) calloc(no_threads, sizeof(size_t));
  thread_work = (thread_work_t *) calloc(no_threads, sizeof(thread_work_t));
  thread_bufs = (thread_buf_t *) calloc(no_threads , sizeof(thread_buf_t));

  file_fd = c_fd;
}

template<typename T>
void init_thread_d_gen(int c_fd){
  init_barrier(&xsync_begin, no_threads);
  init_barrier(&xsync_end, no_threads);

  per_thread_data = (size_t *) calloc(no_threads, sizeof(size_t));
  thread_work = (thread_work_t *) calloc(no_threads, sizeof(thread_work_t));
//  thread_bufs_gen = (thread_buf_t_<T>*) calloc(no_threads , sizeof(thread_buf_t_<T>));

  file_fd = c_fd;
}

void flush_buf(clique_vector* cliques_found, int tid, uint64_t* curr_cliques){
  uint64_t idx_to_add = __sync_fetch_and_add(curr_cliques, thread_bufs[tid].curr);
//  thread_bufs[tid].curr= 0;
//  return;
if(bfs) {
//  if (!out) {
//    printf("MMAPED IO\n");
//    out = 1;
//  }
//  if (idx_to_add * K * sizeof(uint32_t) >= buf_size) {
//    //wait_b(&realloc_barrier);
//    printf("Should not happen %lu found\n", *curr_cliques);
//    exit(1);
//    while (!__sync_bool_compare_and_swap(&_realloc, 0, 1)) {}
//    if (idx_to_add * K * sizeof(uint32_t) < buf_size) {
//      _realloc = 0;
//      goto work;
//    }
//
//    printf("Reallocating\n");
//
//
////    uint64_t to_sync = buf_size;
////    if(to_sync % 4096LU != 0){
////      to_sync = to_sync + 4096LU - to_sync %4096LU ;
////    }
////    uint64_t new_to_sync = buf_size * 2LU;
////    if(new_to_sync % 4096LU != 0){
////      new_to_sync = new_to_sync + 4096LU - new_to_sync %4096LU ;
////    }
////    assert(fallocate(file_fd, 0, 0, new_to_sync) != -1);
////    munmap(cliques_found, to_sync);
////
////      cliques_found= (clique_vector*)mmap(NULL,new_to_sync, PROT_READ|PROT_WRITE, MAP_SHARED, file_fd,0); //cliques_found, to_sync, new_to_sync, MAP_SHARED);
////      if(cliques_found == MAP_FAILED){
////        perror("MAP FAILED");
////        exit(1);
////      }
//////      cliques_found = tmp;
////      buf_size = buf_size * 2L
////              ;
//    _realloc = 0;
//
//    printf("realloc done\n");
//
//  }
  work:

  for (int i = 0; i < thread_bufs[tid].curr; i++) {
    cliques_found[idx_to_add + i] = thread_bufs[tid].buffer[i];
  }
}
else {
//
//  if (!out) {
//    printf("Buffered IO\n");
//    out = 1;
//  }
  size_t b_written = write(file_fd, thread_bufs[tid].buffer, thread_bufs[tid].curr * (sizeof(uint32_t) * K));
  if (b_written == -1) {
    perror("Failed to write\n");
  }
}
//  fsync(file_fd);
  thread_bufs[tid].curr = 0;
}

 void get_work(int tid, thread_work_t* t_work, uint32_t max){
  uint32_t num = (max ) / no_threads;
  uint32_t incr = CHUNK_SIZE;
//  if( num == 0) incr = 1;
  uint32_t idx = __sync_fetch_and_add(&curr_item, incr);

  if(idx >=max) {t_work->start = t_work->stop = max; return;}

  t_work->start = idx;

  t_work->stop = t_work->start + incr;
  if(t_work->stop >max )t_work->stop = max;
}

#endif
