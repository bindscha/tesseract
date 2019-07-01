#ifndef __ALGO_API_H__
#define __ALGO_API_H__
#include"common_include.hpp"
#include<map>
#include<unordered_map>
#include<set>

bool should_be_active(uint32_t v_id);
long unique_patterns[256];
enum DIR_OPT{FWD, BACK};

DIR_OPT DIRECTION = FWD;
//A should be edges or verteices
template<typename A, class T>
bool exec_filter(bool(*func)(A, T , uint32_t), A v_id, T set,  uint32_t step){
  return func(v_id, set, step);
}
template<typename A, class T>
bool exec_expand(bool(*func)(A, T , uint32_t), A v_id, T set,  uint32_t step){
  return func(v_id, set, step);
}
template< class T>
void exec_process(void(*func)( T , uint32_t),  T set,  uint32_t step){
  return func( set, step);
}
template<typename A, class T>
bool exec_filter(bool(*func)(A, T* , uint32_t), A v_id, T* set,  uint32_t step){
  return func(v_id, set, step);
}
template<typename A, class T>
bool exec_expand(bool(*func)(A, T* , uint32_t), A v_id, T* set,  uint32_t step){
  return func(v_id, set, step);
}
template< class T>
void exec_process(void(*func)( T* , uint32_t),  T* set,  uint32_t step){
  return func( set, step);
}
//template<class T>
//  bool filter(T set, uint32_t v_id, int step){}

bool propagation_cond(uint32_t dst,uint32_t src, DIR_OPT DIR){
  switch(DIR){
    case FWD:
      return dst > src;
    case BACK:
      return dst < src;
  }
}
uint32_t mask = 1 <<31;

template<typename T>
inline int match_instance2pattern(const T* set){

//  std::array<uint32_t,K> deg;
//
//  for(int i = 0; i < set->max_size; i++){
//    const uint32_t v_id = set->vertices_in_pattern[i];// |mask ? set->vertices_in_pattern[i]^mask :set->vertices_in_pattern[i];
//
//    deg[i] = 0;
//
//    for(int j =0 ; j <set->no_edges_in_pattern;j++) {
//     const uint32_t src = set->edges_in_pattern[j].src &mask ? set->edges_in_pattern[j].src^mask: set->edges_in_pattern[j].src;
//      if (src == v_id || set->edges_in_pattern[j].dst == v_id) {
//        deg[i]++;
//      }
//    }
//  }
//
//  std::sort(deg.begin(),deg.end());
//
//  int tmp = 0;
//  int i = 0;
//
//  for(const auto &item: deg){
//
//    tmp = tmp | (int)item;
//
//    if(i == K - 1 )break;
//    i++;
//    tmp = tmp << 2;
//  }
//  assert(tmp <= 255);
//
//  return tmp;
  return 0;
}


template<typename T>
inline int match_instance2patternUpdate(T* set){


//  std::array<uint32_t,K> deg;
//
//  int no_edges = 0;
//  for(int i = 0; i < set->max_size; i++){
//    const uint32_t v_id = set->vertices_in_pattern[i];// |mask ? set->vertices_in_pattern[i]^mask :set->vertices_in_pattern[i];
//
//    deg[i] = 0;
//
//    for(int j =0 ; j <set->no_edges_in_pattern;j++) {
//      const uint32_t m_result =  set->edges_in_pattern[j].src & mask;
//      const uint32_t src = m_result ? set->edges_in_pattern[j].src^mask: set->edges_in_pattern[j].src;
//
//
//        if(m_result == 0 &&(src == v_id || set->edges_in_pattern[j].dst == v_id)) {
//          deg[i]++;no_edges++;
//        }
//        else continue;
//      }
//
//  }
//  if(no_edges/2 < (set->max_size -1)) return 0;
//  std::sort(deg.begin(),deg.end());
//
//  int tmp = 0;
//  int i = 0;
//
//  for(const auto &item: deg){
//
//    if(item == 0) return 0;
//    tmp = tmp | (int)item;
//
//    if(i == K - 1 )break;
//    i++;
//    tmp = tmp << 2;
//  }
////  assert(tmp <= 255);
//  return tmp;
  return 0;
}
#endif