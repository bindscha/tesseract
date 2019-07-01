#ifndef __MOTIF_C_HPP__
#define __MOTIF_C_HPP__

#include<set>

int per_thread_patterns[256][256];
//class MotifCounting: public  Algorithm<Pattern<uint32_t, edge_full>> {
//public:
//uint32_t mask = 1 << 31;
//
////__sync_fetch_and_add(&unique_patterns[tmp],1);
//    inline bool filter(uint32_t src, Pattern <uint32_t, edge_full> *set, uint32_t step) {
//      return true;
//    }
//
//    inline bool expand(uint32_t step) {
//      return step < K - 1;
//    }
//     inline void process_update(Pattern <uint32_t, edge_full> *set, uint32_t step) {
//      assert(step == K - 1);
////     const int pattern_id = match_instance2pattern < Pattern < uint32_t, edge_full>>(set);
//
//      int pattern_id2 = match_instance2patternUpdate < Pattern < uint32_t, edge_full>>(set);
//
//       __sync_fetch_and_add(&unique_patterns[set->pattern_id], 1);
//       if (pattern_id2 != 0 && pattern_id2 != set->pattern_id) {
//
//         __sync_fetch_and_sub(&unique_patterns[pattern_id2], 1);
////                        printf("Removing: ");
////                        printPattern(cpy_set, step, n);
////         per_thread_data[tid] -=1;
//       }
//    }
//    inline void process_update_tid(Pattern <uint32_t, edge_full> *set, uint32_t step, int tid) {
//
//
//        const int pattern_id2 = match_instance2patternUpdate < Pattern < uint32_t, edge_full>>(set);
//        set->pattern_id = match_instance2pattern < Pattern < uint32_t, edge_full>>(set);
//       per_thread_patterns[tid][set->pattern_id]++;
//       if (pattern_id2 != 0 && pattern_id2 != set->pattern_id) {
//          per_thread_patterns[tid][pattern_id2]--;
//       }
//    }
//    inline void process(Pattern <uint32_t, edge_full> *set, uint32_t step) {
//      assert(step == K - 1);
//      int pattern_id = match_instance2pattern < Pattern < uint32_t, edge_full>>(set);
//
//      set->pattern_id = pattern_id;
//    }
//    inline void process(Pattern <uint32_t, edge_full> *set, uint32_t step, int tid) {
////          assert(step == K - 1);
//         const int pattern_id = match_instance2pattern < Pattern < uint32_t, edge_full>>(set);
//      set->pattern_id = pattern_id;
//          per_thread_patterns[tid][pattern_id]++;
//
//    }
//
//      inline bool prefilter(uint32_t cand){
//        return true;
//      }
//void activate_nodes(){
//  for(uint32_t i = 0; i < NB_NODES; i++){
//    for(size_t idx = 0; idx < degree[i]; idx++) {
//      active[no_active++] = adj_offsets[i] + idx;
//    }
//  }
//}
//};
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

//template<typename T>
inline void addVertextoSetTS(Pattern<uint32_t,edge_full>* set, uint32_t v, uint32_t step){
#if SYM== 1
  uint32_t dst,ts;
  int j =0 ;
set->vertices_in_pattern[step] = v;
  uint64_t new_edges = set->no_edges_in_pattern;
  uint64_t e1,e2;
  FOREACH_EDGE_TS(v, dst, ts)
     for(int i = 0; i < step ;i++){
      if(dst == set->vertices_in_pattern[i] && ts<= set->ts_max) {
        j++;
        if( ts == set->ts_max ){

//          if(i == 0 && v < i) return;
//          if(i >  0 && (v < set->vertices_in_pattern[1] && v<set->vertices_in_pattern[0]) || (v>set->vertices_in_pattern[1] && v<set->)) {set->reset(new_edges); return;}

              e1 = dst>v?v : dst;// (uint64_t)dst << 32;
              e1 =(e1 << 32 ) |(dst > v? dst:v);

//              e1 = e1 | (dst > v? dst:v);
              e2 = (uint64_t)set->vertices_in_pattern[0] << 32;
              e2 = e2 | set->vertices_in_pattern[1];
              if(e1 < e2 ) {
//                printf("Problem, resetting %u from neigh %u (%u->%u)\n",v,dst,set->vertices_in_pattern[0],set->vertices_in_pattern[1]);
                set->reset(new_edges);
                return;
              }
        }
      set->inc_edge_count();
    }
    }
     if(j==step) break;
  ENDFOR

//  if(new_edges!=
//        set->inc_edge_count();
//  for(int i = 0; i < step -1;i++){
//    uint32_t u = set->vertices_in_pattern[i];
//    if(u > v && has_edge_ts(v, u , ts)) set->inc_edge_count();
//    else
//    if(u < v && has_edge_ts(u,v, ts)) set->inc_edge_count();
//
//  }


#else
  int j = 0;
  uint32_t dst, ts;
  uint64_t new_edges = set->no_edges_in_pattern;
  uint64_t e1,e2;
  set->vertices_in_pattern[step ] = v;
  FOREACH_EDGE_TS(v, dst, ts)
          for(int i =0; i < step; i++){
            uint32_t u = set->vertices_in_pattern[i];
            if(u == dst && ts <= set->ts_max){
              j++;
              if(ts == set->ts_max) {
                e1 = dst>v?v : dst;// (uint64_t)dst << 32;
                e1 =(e1 << 32 ) |(dst > v? dst:v);

//              e1 = e1 | (dst > v? dst:v);
                e2 = (uint64_t)set->vertices_in_pattern[0] << 32;
                e2 = e2 | set->vertices_in_pattern[1];
                if(e1 < e2 ) {
//                printf("Problem, resetting %u from neigh %u (%u->%u)\n",v,dst,set->vertices_in_pattern[0],set->vertices_in_pattern[1]);
                  set->reset(new_edges);
                  return;
                }
                u = u | mask;


              }
              set->add_edge_to_pattern2(u,v);
              break;
            }
          }
  if (j == step )break;
  ENDFOR


#endif


}
inline void addVertextoSet(Pattern<uint32_t,edge_full>* set, uint32_t v, uint32_t step){

  set->vertices_in_pattern[step] = v;
  ++set->no_vertices_in_pattern;

  uint32_t dst;
#if SYM == 1
//  int i=0;
int j = 0;
set->inc_edge_count();
  FOREACH_EDGE_BACK(v, dst)

    for(int i = 0; i <step-1; i++){
      if(dst == set->vertices_in_pattern[i]){
        set->inc_edge_count();
        j++;
        break;
      }
    }
    if(j == step - 1) break;
  ENDFOR
#else
       FOREACH_EDGE(v, dst)
              for(int i = 0; i < step; i++) {

                if (dst == set->vertices_in_pattern[i]) {

                  set->add_edge_to_pattern2(dst, v);//edge_full(src,v));
                  break;

                }
              }
      ENDFOR
#endif



}
#endif
