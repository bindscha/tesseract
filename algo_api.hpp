#ifndef __ALGO_API_H__
#define __ALGO_API_H__
#include"common_include.hpp"
#include<map>
#include<unordered_map>
#include<set>

bool should_be_active(uint32_t v_id);
long unique_patterns[256];

//A should be updates or verteices
template<typename A, class T>
bool exec_filter(bool(*func)(A, T , uint32_t), A v_id, T set,  uint32_t step){
  return func(v_id, set, step);
}


#endif