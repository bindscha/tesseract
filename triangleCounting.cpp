
#include "engine.hpp"

size_t discarded = 0;

//#define K 2
/*
 * Algo data
 *
 */

//Pattern<uint32_t, edge_full>* cliques_found; //this is mmaped to a file; TODO it could be done a bit better





void activate_nodes(){
//
//  if(EXPLORE_MODE== MIDDLE) {
//    no_active = NB_EDGES;
//    return;
//  }
  for(uint32_t i = 0; i < NB_NODES; i++){

      for(size_t idx = 0; idx < degree[i]; idx++) {
        if(edges_ts[adj_offsets[i] + idx].dst < i) continue;
          active[no_active++] = adj_offsets[i] + idx;
      }
  }


}


int main(int argc, char** argv)  {
//  thread_bufs_gen = (thread_buf_t_<Pattern<uint32_t, edge_full>>*) malloc(no_threads * sizeof(thread_buf_t_<Pattern<uint32_t, edge_full>>));
//  for(int i = 0; i < no_threads;i++){
//    thread_bufs_gen[i].curr =0 ;
//    for(int j = 0; j < 2048; j++)
//      thread_bufs_gen[i].buffer[i].alloc_vertices(max);
//  }
SYMMETRIC = true;
  Engine< edge_full>* e = new Engine< edge_full>();
  int c;
  std::string tmp;
  while ((c = getopt(argc, argv, "f:d:vmein:t:b:")) != -1) {
    switch (c) {
      case 'f':
        input_file = optarg;
        break;
      case 'd':
        degree_file = optarg;
        break;
      case 'v':
        EXPLORE_MODE = VERTEX; //exp_mode[atoi(optarg)];
        break;
      case 'm':
        EXPLORE_MODE = MIDDLE; //exp_mode[atoi(optarg)];
        break;
//      case 'i':
//        indexed = true; //exp_mode[atoi(optarg)];
        break;
      case 'n':
        tmp = std::string(optarg);
        NB_NODES = stoul(tmp);
        break;
      case 't':
        tmp = std::string(optarg);
        no_threads = stoul(tmp);
        break;
      case 'b':
        tmp = std::string(optarg);
        e->batch_size= stoul(tmp);
        break;
    }
  }


  no_active_next = 0;

  printf("K %d\n",K);

//  thread_bufs_gen = (Pattern<uint32_t, edge_full> *) malloc(sizeof(Pattern<uint32_t, edge_full> ) * 2048);
  DIRECTION = FWD;



  init();
  init_thread_d(clique_fd);
  e->algo = new TriangleCount();
  e->init_active(&activate_nodes);
  printf("no _active = %u\n",no_active);
  auto start = std::chrono::high_resolution_clock::now();
  e->execute_app();
  printf("Number active %lu, %d - cliques %lu\n", no_active,K,no_triangles);
  e->cancel_compute();

//  printf("All done\n");
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << "Finish triangle counting. Running time : " << diff.count() << " s\n";

//  fsync(clique_fd);
}









