#ifndef __FILTER_CACHE_HPP__
#define __FILTER_CACHE_HPP__
#include "graph_cache.hpp"

template <typename T,typename A>
class FilterCache:public GraphCache<T>{

    int no_k_neigh = 2; //Number of neighbours to put in filter cache
    A* algo;
public:
    FilterCache(){}
    FilterCache(int k_neigh_size, size_t capacity, A* _algo):GraphCache<uint32_t>(capacity),no_k_neigh(k_neigh_size) { 
      algo = _algo;
    }


    void filter_k_neighbours(const edge_full* u_buffer,const size_t items, uint32_t ts){

      for(size_t i = 0; i <items; i++){

        edge_full e = u_buffer[i];
        for(int n = 0; n < no_k_neigh; n++){
          uint32_t dst = edges[adj_offsets[e.src] +n ].dst;
          if((*this)[dst].valid() && (*this)[dst][0] < ts){
            this->invalidate(dst);
          }
          if(!(*this)[dst].valid() && algo->prefilter(dst))
            this->append_to_vertex(e.src,e.src); //the per vertex cache has only 1 item
        }
      }
    }
    void filter_degree(  edge_full* u_buffer, const size_t items, uint32_t ts){

      for(size_t i = 0; i <items; i++){

        edge_full e = u_buffer[i];
        if((*this)[e.src].valid() && (*this)[e.src][0] < ts ){
            this->invalidate(e.src);
        }
        if(!(*this)[e.src].valid() && !algo->prefilter(e.src))
          this->append_to_vertex(e.src,ts);

        if((*this)[e.dst].valid() && (*this)[e.dst][0] < ts ){
          this->invalidate(e.dst);
        }
        if(!(*this)[e.dst].valid() && !algo->prefilter(e.dst))
          this->append_to_vertex(e.dst,ts);
      }
    }
};

#endif