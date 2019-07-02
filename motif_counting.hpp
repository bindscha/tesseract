#ifndef __MOTIF_C_HPP__
#define __MOTIF_C_HPP__

#include<set>

int per_thread_patterns[256][256];
class MotifCountingE {
public:
uint32_t mask = 1 << 31;

//__sync_fetch_and_add(&unique_patterns[tmp],1);
    inline bool filter(const Embedding<uint32_t>*embedding)const  {
      return true;
    }

    inline bool expand(const uint32_t step) const {
      return step < K - 1;
    }
     inline void process_update(Embedding<uint32_t> *pattern, uint32_t step) {

    }
    inline void process_update_tid(const Embedding<uint32_t> *embedding, const uint32_t step, const int tid) {

//      std::array<uint32_t,K> deg;
//      std::array<uint32_t,K> deg2;
//
//      for(int i = 0; i < embedding->no_vertices(); i++){
//        deg[i] = embedding->vertex_degree_at_index(i);
//#ifdef EDGE_TIMESTAMPS
//        deg2[i] = embedding->old_vertex_degree_at_index(i);
//#endif
//      }
//
//      std::sort(deg.begin(),deg.end());
//      std::sort(deg2.begin(),deg2.end());
//
//      int pattern_id1 = 0;
//      int i = 0;
//
//      for(const auto &item: deg){
//
//        pattern_id1 = pattern_id1 | (int)item;
//
//        if(i == embedding->no_vertices() - 1 )break;
//        i++;
//        pattern_id1 = pattern_id1 << 2;
//      }
//       int pattern_id2= 0;
//        i = 0;
//        int no_edg = 0;
//      for(const auto &item: deg2){
//        no_edg+= item;
//        if(item == 0) { pattern_id2 = 0; break;}
//        pattern_id2 = pattern_id2 | (int)item;
//
//        if(i == embedding->no_vertices() - 1 )break;
//        i++;
//        pattern_id2 = pattern_id2 << 2;
//      }
//      if(no_edg/2 < (embedding->no_vertices()-1)) pattern_id2 = 0;
//        per_thread_patterns[tid][pattern_id1]++;
//
//       if (pattern_id2 != 0 && pattern_id2 != pattern_id1) {
//          per_thread_patterns[tid][pattern_id2]--;
//       }
    }
inline void process(const Embedding<uint32_t> *emb, const uint32_t step,const int tid)   {
//
//      std::array<uint32_t,K> deg;
//      for(int i = 0; i < emb->no_vertices(); i++){
//      deg[i] = emb->vertex_degree_at_index(i);
//      }
//      std::sort(deg.begin(),deg.end());
//      int pattern_id1 = 0;
//      int i = 0;
//      for(const auto &item: deg){
//      pattern_id1 = pattern_id1 | (int)item;
//      if(i == emb->no_vertices() - 1 )break;
//      i++;
//      pattern_id1 = pattern_id1 << 2;
//      }
//      per_thread_patterns[tid][pattern_id1]++;

    }

      inline bool prefilter(const Embedding<uint32_t>* embedding,const uint32_t cand ) const {
        return true;
    }
    static void activate_nodes() {
      no_active = 0;
      for(uint32_t i = 0; i < NB_NODES; i++){
        for(size_t idx = 0; idx < degree[i]; idx++) {
          if(edges_full[adj_offsets[i] + idx].dst > i) {
            active[no_active] = adj_offsets[i] + idx;
            no_active++;
          }
        }
      }
    }

    inline void output()const{
      int unique = 0;
      for(int i = 0; i <256; i++){
        for(int j = 0; j <56;j++) {

          unique_patterns[i]+= per_thread_patterns[j][i];

        }
        if (unique_patterns[i] != 0) {
          unique++;
          printf("u[%d]=%lu  ", i, unique_patterns[i]);
        }
      }
      printf("\nFOund %d unique patterns\n", unique);
    }
};
#endif
