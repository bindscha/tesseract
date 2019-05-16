#ifndef __PATTERN_HPP__
#define __PATTERN_HPP__


#include <array>
#include <iostream>
#include <set>





template<typename V, typename E>
class Pattern{
public:
    int max_size=0;
    uint32_t mask = 1 << 31;
    V* vertices_in_pattern;

    E* edges_in_pattern;

    uint32_t ts_max = 0;
    uint32_t no_vertices_in_pattern = 0;
    uint32_t no_edges_in_pattern = -1U;
    uint64_t curr_edge_buf_size = 0;//-1U;

    Pattern(int _max_size){
      max_size = _max_size;
      vertices_in_pattern = (V*) calloc(max_size, sizeof(V));
    }
    Pattern(){}

    volatile int pattern_id;
    void reset() { no_edges_in_pattern = 0;}
    void reset(uint64_t new_num_edges ){
      no_edges_in_pattern = new_num_edges;
    }
    void reset(uint64_t new_num_edges, uint32_t new_num_vertices){
        no_edges_in_pattern = new_num_edges;
        no_vertices_in_pattern = new_num_vertices;
    }

    void add_edge_to_pattern2(uint32_t src, uint32_t dst){//} edge){
      if(no_edges_in_pattern == -1U){
        curr_edge_buf_size = max_size;
        edges_in_pattern = (E*) calloc(max_size , sizeof(E));
        no_edges_in_pattern = 0;
      }

      if(no_edges_in_pattern == curr_edge_buf_size){//} && no_edges_in_pattern != 0){
        edges_in_pattern = (E*) realloc( edges_in_pattern, curr_edge_buf_size * 2 * sizeof(E));
        curr_edge_buf_size *= 2;
      }


      edges_in_pattern[no_edges_in_pattern].src = src;
      edges_in_pattern[no_edges_in_pattern++].dst = dst;

      if(no_edges_in_pattern >curr_edge_buf_size){
        printf("PROBLEM %u \n", no_edges_in_pattern);
      }

    }

    Pattern(const Pattern& other) {
        max_size = other.max_size;
        no_vertices_in_pattern = other.no_vertices_in_pattern;
        no_edges_in_pattern = other.no_edges_in_pattern;

        vertices_in_pattern = (V *) calloc(max_size, sizeof(V));
        edges_in_pattern = (E *) calloc(no_edges_in_pattern, sizeof(E));
        memcpy(vertices_in_pattern, other.vertices_in_pattern, no_vertices_in_pattern * sizeof(V));
        // This blows up when edges are not stored
        //memcpy(edges_in_pattern, other.edges_in_pattern, no_edges_in_pattern * sizeof(V));
    }

    void create(Pattern* copy, uint32_t exl_src, uint32_t exl_dst){
      vertices_in_pattern = (V*) calloc(copy->max_size, sizeof(V));
      max_size =0;
//      for(int i = 0; i < copy->max_size;i++){
//        vertices_in_pattern[i] = copy->vertices_in_pattern[i];
//      }
//      no_edges_in_pattern = copy->no_edges_in_pattern;
      std::set<uint32_t> vertices;
      edges_in_pattern = (E*) calloc(copy->no_edges_in_pattern , sizeof(E));
      no_vertices_in_pattern = copy->no_vertices_in_pattern;
      no_edges_in_pattern = 0;
      for(uint32_t i = 0; i < copy->no_edges_in_pattern; i++){
        uint32_t src = copy->edges_in_pattern[i].src & mask;
        uint32_t dst = copy->edges_in_pattern[i].dst;

        if(src == 0) {
          src = copy->edges_in_pattern[i].src;
          edges_in_pattern[no_edges_in_pattern].src = src;
          edges_in_pattern[no_edges_in_pattern++].dst = dst;
          vertices.insert(src);
          vertices.insert(dst);
        }
        else continue;
//        else src = set->edges_in_pattern[i].src - mask;
//        if((src == exl_src && dst == exl_dst)  || (dst == exl_src && src == exl_dst)) continue;

      }
      for(uint32_t v:vertices){
        vertices_in_pattern[max_size++] = v;
      }
//      printf("new maxsize %d\n",max_size);
    }
    Pattern(Pattern* copy, uint32_t exl_src, uint32_t exl_dst){
      vertices_in_pattern = (V*) calloc(copy->max_size, sizeof(V));
      max_size =0;

      std::set<uint32_t> vertices;
      edges_in_pattern = (E*) calloc(copy->no_edges_in_pattern , sizeof(E));
      no_vertices_in_pattern = copy->no_vertices_in_pattern;
      no_edges_in_pattern = 0;
      for(uint32_t i = 0; i < copy->no_edges_in_pattern; i++){
        uint32_t src = copy->edges_in_pattern[i].src & mask;
        uint32_t dst = copy->edges_in_pattern[i].dst;

        if(src == 0) {
          src = copy->edges_in_pattern[i].src;
          edges_in_pattern[no_edges_in_pattern].src = src;
          edges_in_pattern[no_edges_in_pattern++].dst = dst;
          vertices.insert(src);
          vertices.insert(dst);
        }
          else continue;
      }
      for(uint32_t v:vertices){
        vertices_in_pattern[max_size++] = v;
      }

    }
    inline void inc_edge_count(){

      no_edges_in_pattern++;
    }
//    inline bool is_v_in_pattern(V v_id){
//      for(int i = 0; i < max_size; i++){
//        if(v_id == vertices_in_pattern[i]) return true;
//      }
//      return false;
//
//    }
   inline bool is_v_in_pattern(V v_id,int step){
      for(int i = 0; i < step; i++){
        if(v_id == vertices_in_pattern[i]) return true;
      }
      return false;

    }

    void alloc_vertices(int _max_size) {
      max_size = _max_size;
      vertices_in_pattern = (V*) calloc(max_size, sizeof(V));
    }
    bool operator==(Pattern p1){
       return no_edges_in_pattern == p1.no_edges_in_pattern;
    }
    void operator=(Pattern cpy) {
      int idx = 0;
      if(max_size == 0){
        max_size = cpy.max_size;
        alloc_vertices(max_size);
      }
      for (int idx = 0; idx < max_size; idx++) {
        vertices_in_pattern[idx] = cpy.vertices_in_pattern[idx];
      }

      no_vertices_in_pattern = cpy.no_vertices_in_pattern;
      //this is how it should really be but since we just count we will jsut set the count of edges in the new pattern
      no_edges_in_pattern = cpy.no_edges_in_pattern;
//      for(uint32_t i = 0; i <cpy.no_edges_in_pattern;i++){
//        add_edge_to_pattern2(cpy.edges_in_pattern[i].src, cpy.edges_in_pattern[i].dst);
//      }


    }




};

#endif