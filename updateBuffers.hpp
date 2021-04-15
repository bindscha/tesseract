#ifndef __UPDATE_BUF__
#define __UPDATE_BUF_
#include <random>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <bitset>
uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed)
{
  uint32_t h = seed;
  if (len > 3) {
    const uint32_t* key_x4 = (const uint32_t*) key;
    size_t i = len >> 2;
    do {
      uint32_t k = *key_x4++;
      k *= 0xcc9e2d51;
      k = (k << 15) | (k >> 17);
      k *= 0x1b873593;
      h ^= k;
      h = (h << 13) | (h >> 19);
      h = h * 5 + 0xe6546b64;
    } while (--i);
    key = (const uint8_t*) key_x4;
  }
  if (len & 3) {
    size_t i = len & 3;
    uint32_t k = 0;
    key = &key[i - 1];
    do {
      k <<= 8;
      k |= *key--;
    } while (--i);
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    h ^= k;
  }
  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

class UpdateBuffer{

    std::vector<bool>* node_bitset;
    x_barrier xsync_begin, xsync_end;

    size_t no_up_currently  =0;
    uint32_t batch_size = 1000;
public:


    edge_full* updates;
    x_barrier updates_ready;
    x_barrier updates_consumed;
    uint32_t curr_batch_start = 0;
    uint32_t curr_batch_end = 0;
    uint32_t curr_ts = 0;
    size_t initial_chunk = 0;
    int no_threads = 1;

    bool shuffle = true;//false;//false;

    static size_t f (){
        static int i = 0;
        return i++;
    }
//    void shuffle_edge_idx(size_t _NB_EDGES){
////      v.reserve(_NB_EDGES - initial_chunk);
//        std::generate(v.begin(), v.end(), UpdateBuffer::f);
////        std::iota(std::begin(v), std::end(v), 0);
//        std::random_device rd;
//        std::mt19937_64 g(rd());
//
//      std::shuffle(v.begin(), v.end(), g);
////
//    }
//    uint32_t* node_ts;
    size_t no_edges;

    UpdateBuffer(uint32_t _batch_size, uint64_t _NB_EDGES, uint32_t nb_nodes, size_t _initial_chunk = 0, int n = 1)
            : batch_size(_batch_size), no_edges(_NB_EDGES), initial_chunk(_initial_chunk), no_threads(n){//} v(_NB_EDGES) {
      //      node_bitset = (std::vector<bool> *) calloc(nb_nodes, sizeof(std::vector<bool>));
      init_barrier(&updates_ready, 2);
      init_barrier(&updates_consumed, 2);
      init_barrier(&xsync_end, no_threads);
      init_barrier(&xsync_begin, no_threads);
//      memset(degree, 0 , /)
      if(!initial_chunk) {
        printf("Setting degrees to 0\n");
        for (uint32_t i = 0; i < nb_nodes; i++) {
          degree[i] = 0;
        }
      }

      updates = (struct edge_full*) calloc(batch_size , sizeof(edge_full)); // TODOto replace this should be only sizeof BATCH and we have perhaps two buffers,
      // one for accumulating updates while updates from other are processed
      uint64_t i = 0;
      uint64_t equals = 0;
      // This was initially used to shuffle updates but was moved to tesseract_Driver.cpp
//      if(shuffle)
//        shuffle_edge_idx(_NB_EDGES);

//      for(size_t i  = 0; i <no_edges;i++){
//        printf("%u->%u (%u->%u)\n",updates[i].src,updates[i].dst, e[i].src,e[i].dst);
//      }
//      printf("added %lu updates(%lu self loops) \n",no_edges,equals);
//      batch_size= 100000;
//      batch_size=no_edges - 10000000;
    }
    ~UpdateBuffer(){
      free(updates);
    }
    inline void incNoUpdates(){
        no_up_currently++;
    }
    inline void resetNoUpdates(){
        no_up_currently = 0;
    }
    size_t preload_edges_before_update(edge_full* e, int tid, edge_ts* graph_edges, int no_threads, const std::unordered_set<uint64_t>&update_idx){
        wait_b(&xsync_begin);
      size_t no_edges = NB_EDGES;
      size_t num = no_edges / no_threads;

      size_t start = (size_t)tid *num;
      size_t stop = start + num;
      if(tid == no_threads - 1) stop =  no_edges;
        size_t total_looped = 0;
//      for(;start < stop && total_looped  < NB_EDGES - 10000000;total_looped++){
          for(;start < stop && total_looped  < no_edges ;total_looped++){
              if(update_idx.count(total_looped) != 0 ) continue;
        if(e[total_looped].src >e[total_looped].dst ){//->at(total_looped)].dst) {
            continue;
        }


        start++;
        uint32_t src = e[total_looped].src;//->at(total_looped)].src;//*vec)[start]].src;
        uint32_t dst = e[total_looped].dst;//->at(total_looped)].dst;

        assert(degree[src]>= 0);
//        if(degree[src] >15402+ 975418){
//            printf("Problemd with node %u - degree is %lu\n",src,degree[src]);
//        }
//        assert(degree[src] <= (15402+ 975418));


        size_t deg = degree[src];//__sync_fetch_and_add(&degree[src],1);
          degree[src]++;
        graph_edges[adj_offsets[src] + deg].src = src;
        graph_edges[adj_offsets[src] + deg].dst = dst;
        graph_edges[adj_offsets[src] + deg].ts = 0;
          assert(degree[dst]>= 0);
//          if(adj_offsets[dst] < NB_EDGES) {
              deg = degree[dst]; //__sync_fetch_and_add(&degree[dst],1);
              degree[dst]++;
//          if(degree[dst] >15402+ 975418){
//              printf("Problemd with node %u - degree is %lu\n",dst,degree[dst]);
//          }
//          assert(degree[dst] <= (15402+ 975418));
//          assert(degree[0] <= (15402+ 975418));
              graph_edges[adj_offsets[dst] + deg].src = dst;
              graph_edges[adj_offsets[dst] + deg].dst = src;
              graph_edges[adj_offsets[dst] + deg].ts = 0;
//          }
      }
      wait_b(&xsync_end);
      return total_looped;

//      curr_batch_end = initial_chunk;
//      assert(degree[NB_NODES -1] == NB_EDGES - adj_offsets[NB_NODES - 1]);
    }

    inline size_t get_no_updates() const{
      return no_up_currently;
    }

    inline bool has_work()const{
      return curr_batch_end < no_edges;
    }
    void update_graph_structure(edge_ts* graph_edges,edge_full* in_stream, int w_id, int no_workers){
//      curr_batch_start = curr_batch_end;
//
//      no_up_currently=0;
//
//      while(no_up_currently < batch_size &&  curr_batch_end < no_edges){
//
//        uint32_t src = in_stream[v[curr_batch_end]].src;
//        uint32_t dst = in_stream[v[curr_batch_end]].dst;
//        curr_batch_end++;
//        if(src> dst) {continue;}
//
//          graph_edges[adj_offsets[src] + degree[src]].src = src;
//          graph_edges[adj_offsets[src] + degree[src]].dst = dst;
//          graph_edges[adj_offsets[src] + degree[src]].ts = curr_ts;
////          node_ts[src] = curr_ts;
//          degree[src]++;
//
//          graph_edges[adj_offsets[dst] + degree[dst]].src = dst;
//          graph_edges[adj_offsets[dst] + degree[dst]].dst = src;
//          graph_edges[adj_offsets[dst] + degree[dst]].ts = curr_ts;
////          node_ts[dst] = curr_ts;
//          degree[dst]++;
////             if( !(src %(no_workers/2) == tid % (no_workers/2) && (dst%2 == (tid/(no_workers)) % 2) ) )continue;
////             printf("TID %d processing %u (%u-%u)\n",tid,i,src,dst);
////             __sync_fetch_and_add(&curr_item,1);
//        //             uint64_t e = (uint64_t) src << 32;
////             e = e | dst;
////             if(!(e % no_workers == tid))continue;
//        uint32_t h_src =murmur3_32(( uint8_t *)(&src), 4, dst);
////        if((true)){//
//          if(h_src % no_workers == w_id ) {
//            this->updates[no_up_currently].src = src;
//            this->updates[no_up_currently].dst = dst;
//            no_up_currently++;
//          }
//
//      }
//        curr_ts++;

    }

};

#endif