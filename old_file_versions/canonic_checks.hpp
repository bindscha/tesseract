#ifndef __CANONIC_CHEKS_HPP__
#define __CANONIC_CHEKS_HPP__

#include "Pattern.hpp"

inline bool check_in_set(uint32_t src, Pattern<uint32_t, edge_full>* set, uint32_t step){
  for (uint32_t i = 0; i <step; i++){
    if (set->vertices_in_pattern[i] == src) return true;
  }
  return false;
}
template<typename T>
inline bool canonic_check_r2_new(uint32_t v,const Embedding<T> * embedding, uint32_t step){
  uint32_t i = 0;
  bool foundNeighbour = false;
  uint32_t n_ts = embedding->max_ts();
  for(size_t i = 0; i < embedding->no_vertices(); ++i) {
    uint32_t u = (*embedding)[i];
    if(!foundNeighbour && has_edge_ts(u, v, embedding->max_ts(), &n_ts)) foundNeighbour = true;
    else
      if(foundNeighbour && u > v &&(( n_ts < embedding->max_ts() && u != (*embedding)[1] ) || n_ts == embedding->max_ts())) return false;
  }
  return true;
}

inline bool canonic_check_r2_newP(uint32_t src, Pattern<uint32_t ,edge_full>* set, uint32_t step){
  uint32_t i = 0;
  bool foundNeighbour = false;
  uint32_t n_ts = set->ts_max;
  for(;i < step; i++){

    uint32_t dst = set->vertices_in_pattern[i];
    if (!foundNeighbour &&  has_edge_ts(dst,src, set->ts_max, &n_ts)) foundNeighbour = true;
    else
    if((foundNeighbour && dst > src ) && ((n_ts < set->ts_max && dst != set->vertices_in_pattern[1] ) || n_ts == set->ts_max)) return false;
//      if(foundNeighbour){
//        if(n_ts == set->ts_max && dst > src) return false;
//        if(has_edge_ts(dst, src, set->ts_max, &n_ts) && n_ts == set->ts_max  && src > )
//      }


  }
  return true;

}
inline bool canonic_check_r2_new_nodets(uint32_t v, Pattern<uint32_t ,edge_full>* set, uint32_t step, uint32_t* node_ts){
  uint32_t i = 0;
  bool foundNeighbour = false;
  uint32_t n_ts = set->ts_max;
  uint32_t tv = node_ts[v];
  uint32_t t0 = set->ts_max;
  if(tv > t0 || (tv == t0 && v<set->vertices_in_pattern[0])) return false;
  for(;i < step; i++){
    uint32_t u = set->vertices_in_pattern[i];
    if(!foundNeighbour && has_edge_ts(u,v,set->ts_max,&n_ts)) foundNeighbour = true;
    else
      if(foundNeighbour && u > v ) {
        uint32_t tmp_ts;
        if(has_edge_ts(u,v,set->ts_max,&tmp_ts) && tmp_ts == set->ts_max &&  u < set->vertices_in_pattern[0]) return false;
        else
          if(node_ts[u] < t0 && u != set->vertices_in_pattern[1]) return false;
      }
  }
  return true;


}
inline bool canonic_check_r1(uint32_t src,  Pattern<uint32_t ,edge_full>* set, uint32_t step){
  return   set->vertices_in_pattern[0] < src;// && !set->is_v_in_pattern(src,step);
}
inline bool canonic_check_r1E(const uint32_t u,const Embedding<uint32_t>* embedding, const uint32_t step){
  return  (*embedding)[0] < u;
}

inline bool canonic_check_r2(uint32_t src, Pattern<uint32_t ,edge_full>* set, uint32_t step){
  uint32_t i = 0;
  bool foundNeighbour = false;
  for(;i < step; i++){
    uint32_t dst = set->vertices_in_pattern[i];
    if (!foundNeighbour &&has_edge(src,dst)) foundNeighbour = true;
    else
    if(foundNeighbour && dst > src) return false;
  }
  return true;

}
inline bool canonic_check_r2E(const uint32_t v,const Embedding<uint32_t>* embedding, const uint32_t step)  {
  uint32_t i = 0;
  bool foundNeighbour = false;
    for(; i < embedding->no_vertices(); ++i) {
      const uint32_t u = (*embedding)[i];
    if (!foundNeighbour && has_edge(u,v)) foundNeighbour = true;
    else
    if(foundNeighbour && u > v) return false;
  }
  return true;

}
//Most likely first version for edge patterns
bool canonic_check(uint32_t src, Pattern<uint32_t ,edge_full>* set, uint32_t step, uint32_t parent){

  if( src <= set->vertices_in_pattern[0] ||set->is_v_in_pattern(src,step)) return false;

  int found = 0;
  int i = 0;
  uint32_t min_n = NB_NODES;
  int first_n = 0;
  for(; i < step; ++i){
    uint32_t dst;

    FOREACH_EDGE(src, dst)
    if(set->vertices_in_pattern[i] == dst) {
      if(!found) first_n = i;
      found++;
      if(set->vertices_in_pattern[i] < min_n ) min_n = set->vertices_in_pattern[i];


    }

    ENDFOR
//    if(found) break;
  }

  if(found!= 1 && min_n != parent) return false; //set->vertices_in_pattern[step -1]) return false;

//  if(i == step) return false;
//  i++;
  i = first_n;
  i++;
  for(;i<step;++i){
    if(set->vertices_in_pattern[i] >= src) return false;
  }
  return true;
}
bool canonic_check_v(uint32_t src, Pattern<uint32_t ,edge_full>* set, uint32_t step){
//  if(step == 0) return true;
  if( src <= set->vertices_in_pattern[0] ||set->is_v_in_pattern(src,step)) return false;

  int found = 0;
  uint32_t i = 0;


  for(; i < step; ++i){
    uint32_t dst;

    FOREACH_EDGE(src, dst)
    if(!found&& set->vertices_in_pattern[i] == dst) {
      found = 1; break;

    }

    ENDFOR

    if(found) break;
  }

  if(i == step) return false;
  i++;
  for(;i<step;++i){
    if(set->vertices_in_pattern[i] >= src) return false;
  }
  return true;
}

#endif