#ifndef __CLIQUES_OOC_HPP__
#define __CLIQUES_OOC_HPP__


//Clique finding filter functions

inline bool should_be_active(uint32_t v_id){
  return degree[v_id] >= K -1;
}


//optional additional pruning for all-to-all conncetivity
//inline bool is_in_set(clique_vector set, uint32_t v_id, uint32_t step){
//
//  for(uint32_t i = 0; i < step; i++){
//    if(v_id == set.buffer[i] ) return true;
//  }
//  return false;
//}
//
//bool canonic_check(uint32_t src, clique_vector set, uint32_t step){
//  if(step != 0 && set.buffer[0] >= src) return false;
//  int count = 0;
//  if(is_in_set(set,src,step)) return false;
//  bool found = false;
//  for( int i = 0; i < step; i++){
//    uint32_t dst;
//    FOREACH_EDGE(src, dst)
//            if(!found && set.buffer[i] == dst && should_be_active(dst)) {found = true;  }
//            else
//              if (found && set.buffer[i] > src) return false;
//            if(set.buffer[i] == dst) if(count< step) count++;
//    ENDFOR
//
//  }
//  return count == step;
//}
//bool filter(uint32_t src, clique_vector set, uint32_t step) {
//  uint32_t count = 0;
//  uint32_t dst;
//  FOREACH_EDGE(src, dst)
//  if (should_be_active(dst) && is_in_set(set, dst, step)) count++;
//  if (count == step) break;
//  ENDFOR
//
//  return count == step;
//
//}

void process(Pattern <uint32_t, edge_full> *set, uint32_t step) {}

bool expand(uint32_t src, Pattern <uint32_t, edge_full> *set, uint32_t step) {
  return step < K - 1;
}

class CliqueFind: public  Algorithm<Pattern<uint32_t, edge_full>> {

      inline bool is_in_set(Pattern<uint32_t, edge_full> set, uint32_t v_id, uint32_t step){

        for(uint32_t i = 0; i < step; i++){
          if(v_id == set.vertices_in_pattern[i] ) return true;
        }
        return false;
      }

      inline bool filter(uint32_t src, Pattern<uint32_t, edge_full>* set, uint32_t step) {

        uint32_t dst;
//        if(step < K - 1) return true;


        return set->no_edges_in_pattern == ((step +1)* (step))/2;
//        if(!should_be_active(src)) return false;
//        uint32_t ts;
//        FOREACH_EDGE_TS(src, dst, ts)
//        if (should_be_active(dst) && is_in_set(*set, dst, step) ) count++;//&&ts< set->ts_max) count++;
//        if (count == step) break;
//        ENDFOR
//
//        return count == step ;

      }

      inline void process(Pattern <uint32_t, edge_full> *set, uint32_t step) {

      }
      inline void process(Pattern <uint32_t, edge_full> *set, uint32_t step,int tid) {

      }
      inline void process_update(Pattern <uint32_t, edge_full> *set, uint32_t step) {

      }
    inline void process_update_tid(Pattern <uint32_t, edge_full> *set, uint32_t step,int tid) {

      }
      inline bool prefilter(uint32_t cand){
        return degree[cand] >=K - 1;
    }

     inline bool expand(uint32_t step) {
        return step < K - 1;
      }

      void activate_nodes(){
        for (uint32_t i = 0; i < NB_NODES; i++) {
          if (prefilter(i))//should_be_active(i))
            for (size_t idx = 0; idx < degree[i]; idx++) {
              if (prefilter(edges_full[adj_offsets[i] + idx].dst) && edges_full[adj_offsets[i] + idx].dst >
                                                                              i)
                active[no_active++] = adj_offsets[i] + idx;
            }
        }
      }
};
class CliqueFindE {//}; public  Algorithm<Embedding<uint32_t>> {
public:
inline bool is_in_set(Embedding<uint32_t> embedding, uint32_t v_id, uint32_t step){

  if(embedding.contains(v_id)) return true;

  return false;
}

inline bool filter( const uint32_t src, const Embedding<uint32_t>* embedding,const uint32_t step)const  {
  return embedding->no_edges()  == ((step +1)* (step))/2;

}

inline void process(Embedding<uint32_t>* embedding, uint32_t step) {

}
inline void process(const Embedding<uint32_t>* embedding, const uint32_t step, const int tid)  {

}
inline void process_update(Embedding<uint32_t>* embedding, uint32_t step) {

}
inline void process_update_tid(const Embedding<uint32_t>* embedding,const uint32_t step,const int tid) {

}
inline bool prefilter(const uint32_t cand) const {
  return degree[cand] >=K - 1;
}
inline bool prefilter(const Embedding<uint32_t>* embedding,const uint32_t cand ) const {



return degree[cand] >=K - 1;
}
inline bool expand(const uint32_t step) const {
  return step < K - 1;
}
void activate_nodes(){
  for (uint32_t i = 0; i < NB_NODES; i++) {
    if (prefilter(i))//should_be_active(i))
      for (size_t idx = 0; idx < degree[i]; idx++) {
        if (prefilter(edges_full[adj_offsets[i] + idx].dst) && edges_full[adj_offsets[i] + idx].dst >
                                                               i)
          active[no_active++] = adj_offsets[i] + idx;
      }
  }
}
    void output(){
      printf("Done counting cliques\n");
    }
};

#endif