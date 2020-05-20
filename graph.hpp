#pragma once
#ifndef __GRAPH_H__
#define __GRAPH_H__
#include "common_include.hpp"

#include <unordered_map>



//#define NB_NODES 100000LU // // 39459925LU //
//#define NB_NODES 4847571LU // // 39459925LU //
extern uint64_t NB_NODES;
extern uint64_t NB_EDGES;
//extern std::bitset<uint32_t>
extern bool SYMMETRIC ;

struct edge{
    uint32_t dst;
    edge(uint32_t v){
       dst = v;
    }
};
struct edge_ts{
    uint32_t src;
    uint32_t dst;
    uint32_t ts;
    edge_ts(uint32_t v){
       dst = v;
    }
};
struct edge_full{
    uint32_t src;
    uint32_t dst;

    edge_full(uint32_t v1,uint32_t v2){
       dst = v2;
           src = v1;
    }
};
//extern struct edge* edges_ts;
extern struct edge_full* edges_full;
extern struct edge_ts* edges;

extern bool do_updates;

#define FOREACH_EDGE_FWD(n, _dst)\
       for(uint32_t _i = 0; _i < degree[n]; _i++){ \
              uint32_t dst2 = edges_full[adj_offsets[n] + _i].dst;\
              if(dst2 > n) _dst = dst2; else continue;

#define FOREACH_EDGE_BACK(n, _dst)\
       for(uint32_t _i = 0; _i < degree[n]; _i++) { \
              uint32_t dst2 = edges[adj_offsets[n] + _i].dst; \
              if(dst2 < n) _dst = dst2; else continue;

#define FOREACH_EDGE(n, _dst)\
       for(uint32_t _i = 0; _i < degree[n]; _i++) { \
            _dst = edges_full[adj_offsets[n] + _i].dst;

#define FOREACH_EDGE_TS(n, _dst, _ts)\
       for(uint32_t _i = 0; _i < degree[n]; _i++) { \
              _dst = edges[adj_offsets[n] + _i].dst;\
              _ts = edges[adj_offsets[n] + _i].ts;

#define ENDFOR }

#define FOREACH_EDGE_TS_BACK(n, _dst, _ts)\
       for(uint32_t _i = 0; _i < degree[n]; _i++) { \
              if(edges[adj_offsets[n] + _i].dst > n) continue; \
              _dst = edges[adj_offsets[n] + _i].dst;\
              _ts = edges[adj_offsets[n] + _i].ts;

#define FOREACH_EDGE_TS_FWD(n, _dst, _ts)\
       for(uint32_t _i = 0; _i < degree[n]; _i++) { \
              if(edges[adj_offsets[n] + _i].dst < n) continue;\
              _dst = edges[adj_offsets[n] + _i].dst; \
              _ts = edges[adj_offsets[n] + _i].ts;


#define ENDFOR }
 bool has_edge_ts(uint32_t src, uint32_t dst,uint32_t ts,uint32_t* ts2);
bool has_edge(uint32_t src, uint32_t dst,uint32_t ts,uint32_t* ts2);
 bool has_edge_sym(const uint32_t src, const uint32_t dst);
 bool has_edge(const uint32_t src, const uint32_t dst);

extern uint32_t *degree;
extern uint32_t *degree_in;
extern size_t* adj_offsets;
extern size_t* adj_offsets_in;
extern uint32_t* active;//[NB_NODES];3
extern uint32_t* active_next;//[NB_NODES];
extern uint64_t no_active, no_active_next;
extern bool* in_frontier;

extern int clique_fd;
extern char* degree_file;
extern char* input_file;
extern char* update_file;


void init_adj_degree();
void init_graph_input(bool _mmap);

#endif
