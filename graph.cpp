#include "graph.hpp"
//#include "parallel_ligra.hpp"

uint64_t NB_NODES;
uint64_t NB_EDGES;

bool SYMMETRIC = false;
//struct edge* edges_ts;
struct edge_full* edges_full;
struct edge_ts* edges;
uint32_t *degree;
uint32_t *degree_in;
size_t* adj_offsets;
size_t* adj_offsets_in;
uint32_t* active;//[NB_NODES];
uint32_t* active_next;//[NB_NODES];
uint64_t no_active, no_active_next;
bool* in_frontier;

int clique_fd;
char* degree_file;
char* input_file;
char* update_file;
bool do_updates = 0;


inline bool has_edge_ts(uint32_t src, uint32_t dst, uint32_t ts,uint32_t* ts2) {
  uint32_t tmp_dst, ts_t;
  FOREACH_EDGE_TS(src, tmp_dst, ts_t)
    if(dst == tmp_dst) {
      if (ts_t <= ts) {
        *ts2 = ts_t;
        return true;
      }
    }
//    if(ts_t > ts) break;
  ENDFOR
  return false;
}

bool has_edge_ts_set(uint32_t src, uint32_t dst,uint32_t* ts2){
  uint32_t tmp_dst, ts_t;
  FOREACH_EDGE_TS(src, tmp_dst, ts_t)
    if(dst == tmp_dst) {
        *ts2 = ts_t;
        return true;

    }

  ENDFOR
  return false;
}
bool has_edge(const uint32_t src, const uint32_t dst){
  uint32_t tmp_dst;

  FOREACH_EDGE(src,tmp_dst)//_BACK(src, tmp_dst)
  if(dst == tmp_dst )
    return true;
  ENDFOR

  return false;
}
bool has_edge_sym(const uint32_t src, const uint32_t dst){
  uint32_t tmp_dst;

  FOREACH_EDGE_BACK(src,tmp_dst)//_BACK(src, tmp_dst)
    if(dst == tmp_dst )
      return true;
  ENDFOR
  return false;
}


void init_adj_degree(){
  int fd = open(degree_file, O_RDONLY, O_NOATIME);
  if(fd == -1) {
    perror("File open failed");
    exit(1);

  }
  adj_offsets = (size_t*) calloc(NB_NODES, sizeof(size_t));
  size_t b_read = read(fd, adj_offsets, NB_NODES * sizeof(size_t));
  assert(b_read == NB_NODES * sizeof(size_t));

if(!do_updates) {
    for (uint32_t i = 0; i < NB_NODES - 1; i++) {
        assert(degree[i] == 0);
        degree[i] = adj_offsets[i + 1] - adj_offsets[i];
    }
    degree[NB_NODES - 1] = NB_EDGES - adj_offsets[NB_NODES - 1];

  uint64_t n_edges = 0;
  for(uint32_t i = 0; i <NB_NODES;i++){
      n_edges += degree[i];
  }
  assert(n_edges == NB_EDGES);
}
}





void init_graph_input(bool _mmap){
  degree = (uint32_t *) calloc(NB_NODES, sizeof(uint32_t));
      degree_in = (uint32_t *) calloc(NB_NODES, sizeof(uint32_t));

  //Open edge file
  int fd = open(input_file, O_RDWR);
  if(fd == -1) {
    perror("Failed to open input file");
    exit(1);
  }
  struct stat sb;
  fstat(fd, &sb);


  NB_EDGES = (size_t) sb.st_size / sizeof(struct edge_full);

  active= (uint32_t*) malloc(NB_EDGES* sizeof(uint32_t));

  if(_mmap){
    edges_full = (struct edge_full*) mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);


    int fd2 = open("/media/nvme/output_f", O_RDWR|O_CREAT|O_NOATIME|O_LARGEFILE);
    fallocate(fd2, 0, 0, NB_EDGES* sizeof(edge_ts));
    if(fd2 == -1) {
      perror("Failed to open out file");
      exit(1);
    }
    edges = (struct edge_ts*) mmap(NULL, NB_EDGES* sizeof(edge_ts), PROT_READ| PROT_WRITE, MAP_SHARED, fd2, 0);
    if(edges == MAP_FAILED){
      perror("Failed mapping");
      exit(1);
    }
    printf("NB_EDGES  %lu \n",NB_EDGES);
  }
  else {
    edges_full = (struct edge_full*) malloc(sb.st_size);
    size_t b_read = 0;
    while(b_read <sb.st_size){
      size_t b_r = read(fd, edges_full+b_read/sizeof(struct edge_full), sb.st_size - b_read);
      assert(b_r!=-1);
      b_read += b_r;
  }
      edges = (struct edge_ts*) calloc(sizeof(edge_ts) ,NB_EDGES);
    if(do_updates)
    for(size_t i =0 ; i < NB_EDGES;i++){
        edges[i].ts = UINT64_MAX;
    }
    printf("NB_EDGES  %lu \n",NB_EDGES);
  }


  init_adj_degree();


  assert(NULL!=edges);
  if(!do_updates) {
//  for(uint32_t i = 0; i <NB_NODES;i++) {
//      for (uint32_t j = 0; j < degree[i]; j++) {
//          uint32_t dst = edges_full[adj_offsets[i] + j].dst;
//          bool found = false;
//          for (size_t k = 0; k < degree[dst]; k++) {
//              if (i == edges_full[adj_offsets[dst] + k].dst) {
//                  found = true;
//                  break;
//              }
//
//          }
//          assert(found);
//      }
//  }
//    printf("Graph is undirected");
//    exit(0);

//TODO PAralelize, too slow for bigger graphs
#pragma omp parallel for num_threads(56)
    for(uint32_t i = 0; i < NB_NODES; i++){
        for(uint32_t j = 0; j < degree[i];j++){

            if(edges_full[adj_offsets[i] + j].src != i){
                printf("Problem with input %u-%u is in edge array, instea of %u-%u\n",edges_full[adj_offsets[i] + j].src , edges_full[adj_offsets[i] + j].dst, i);
            }
      assert(edges_full[adj_offsets[i] + j].src == i);

      edges[adj_offsets[i] + j].src= i;
      edges[adj_offsets[i] + j].dst = edges_full[adj_offsets[i] + j].dst;
      edges[adj_offsets[i] + j].ts = 0;// adj_offsets[i]+ j;
//      if(edges_full[adj_offsets[i]  +j].dst < i){
//        if(! has_edge_ts_set(edges[adj_offsets[i]  +j].dst,i,&edges[adj_offsets[i] + j].ts)){
//          edges[adj_offsets[i] + j].ts = 0;// adj_offsets[i] +j;
//        }
//      }
//      else
//        edges[adj_offsets[i] + j].ts = adj_offsets[i] + j;
      }
    }
  }

  no_active_next = 0;

}


