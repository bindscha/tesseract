#ifndef TRIANGLE_C_HPP
#define TRIANGLE_C_HPP

//#define K 3
//#define SIZE 5083604130LU //triangle uk 2005 directed
//#define SIZE 385730264LU //300LU triangles

//bool triangle_filter(uint32_t src, clique_vector set, uint32_t step){
//  if(step < K - 1) return true;
//
//  for(uint32_t i = 0; i <degree[src];i++){
//    uint32_t dst = edges[adj_offsets[src] + i].dst;
////should have a wrapper to loop over edges in one direction and hide the second condition from the end user
//    if(dst == set.buffer[0]) return true;
//
//  }
//  return false;
//}

class TriangleCount: public  Algorithm<Pattern<uint32_t, edge_full>> {

      bool is_in_set(Pattern<uint32_t, edge_full> set, uint32_t v_id, uint32_t step){

        for(uint32_t i = 0; i < step; i++){
          if(v_id == set.vertices_in_pattern[i] ) return true;
        }
        return false;
      }

      bool filter(uint32_t src, Pattern<uint32_t, edge_full>* set, uint32_t step) {
        if(step < K - 1) return true;
        uint32_t dst,ts;

//        FOREACH_EDGE_TS(src,dst,ts)
//                if(ts <=set->ts_max)
        FOREACH_EDGE_BACK(src,dst)
                if(dst == set->vertices_in_pattern[0] ) return true;
        ENDFOR
        return false;


      }

inline void process(Pattern <uint32_t, edge_full> *set, uint32_t step) {

}
inline void process(Pattern <uint32_t, edge_full> *set, uint32_t step,int tid) {

}
inline void process_update(Pattern <uint32_t, edge_full> *set, uint32_t step) {

}
inline void process_update_tid(Pattern <uint32_t, edge_full> *set, uint32_t step,int tid) {

}
      inline bool prefilter(uint32_t t){
        return true;
      }
      bool expand( uint32_t step) {
        return step < K - 1;
      }
};
#endif