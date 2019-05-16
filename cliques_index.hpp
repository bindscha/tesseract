#ifndef __CLIQUES_INDEX_HPP__
#define __CLIQUES_INDEX_HPP__
class CliqueIndex: public  Algorithm<Pattern<uint32_t, edge_full>> {

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
  return degree[cand] >=K - 2;
}

inline bool expand(uint32_t step) {
  return step < K - 1;
}
};


#endif