//#define SYM 0
/*
 * void explore(uint32_t src, Pattern set, int step, int thread_id){
 *  count = 0
 *  for dst in neighbours(src) //Check if this node is connected to all the other nodes in the set
 *    if dst < src && in_set(set, dst, step) count++
 *    if count == step break
 *
 *  if (count != step) return // if not return
 *
 *  if (step < K )
 *    for dst in neigbours(src)
 *      if dst < src continue
 *      set[step] = src     //Add the source to the set and start exploring from this dst
 *      explore(dst, step, step + 1, tid)
 *
 *  if(step == K && count == step)
 *    set[step] = src
 *    add item in thread local vertices_in_pattern (set)
 *    incr count of cliques
 *  }
 *
 *
 * main (){
 *  parallel_for all src in nodes
 *    set = {src}
 *    for all dst in neighbours(src)
 *      if dst < src
 *         explore (dst, set, 1, tid)
 *
 *
 * }
 *
 *
 *
 *
 *  */
//#include "engine.hpp"
#include "engine_emb.hpp"

size_t discarded = 0;

//#define K 2
/*
 * Algo data
 *
 */

//Pattern<uint32_t, edge_full>* cliques_found; //this is mmaped to a file; TODO it could be done a bit better
//
//void printPattern( Pattern<uint32_t, edge_full>* set,uint32_t step, uint32_t v_id){
//
//        printf("\n");
//        for(int i =0;i < step; i++){
//          if(i!=0)
//            printf("(%u)%u ", v_id,set->vertices_in_pattern[i]);
//          else
//            printf("%u ", set->vertices_in_pattern[i]);
//        }
//        printf("(%u)%u\n",v_id,set->vertices_in_pattern[step]);
//
//}



void activate_nodes(){
if(EXPLORE_MODE == MIDDLE){
  for(uint32_t i = 0; i < NB_NODES; i++){
    for(size_t idx = 0; idx < degree[i]; idx++) {
      active[no_active++] = adj_offsets[i] + idx;
    }
  }
//  no_active = NB_EDGES;
  assert(no_active == NB_EDGES);
  return;
}
  for(uint32_t i = 0; i < NB_NODES; i++){
    for(size_t idx = 0; idx < degree[i]; idx++) {
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
  SYMMETRIC = false;
//  Engine< edge_full>* e = new Engine< edge_full>();

  int c;
  std::string tmp;
  int set_K = 3;
  uint64_t b_size,initial_chunk = 0;
  while ((c = getopt (argc, argv, "f:d:vmen:t:b:uk:c:")) != -1) {
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
        b_size= stoul(tmp);
        break;
      case 'u':
        do_updates = 1;
        break;
      case 'k':
//        K = atoi(optarg);
        break;
      case 'c':
        tmp = std::string(optarg);
        initial_chunk = stoul(tmp); //preload initial_chunk edges before starting to apply updates
        break;
    }
  }

  Engine< Embedding<uint32_t>>* e = new Engine<Embedding<uint32_t>>(b_size,initial_chunk);
  no_active_next = 0;
  auto start = std::chrono::high_resolution_clock::now();
  ;

//  thread_bufs_gen = (Pattern<uint32_t, edge_full> *) malloc(sizeof(Pattern<uint32_t, edge_full> ) * 2048);
  DIRECTION = FWD;



  init();
  init_thread_d(clique_fd);

  memset(unique_patterns,0, 256* sizeof(int));
  for(int i = 0; i < no_threads;i++){
    memset(per_thread_patterns[i],0, 256*sizeof(int));
  }
//  e->algo = new MotifCounting();
  e->algo = new MotifCountingE();
//  e->K = set_K;
  printf("Looking for %d-motifs...\n", K);
  if(!do_updates)
   e->init_active();
  e->execute_app();



  printf("no _active = %u\n",no_active);


  printf("Number active %lu, motifs %lu\n", no_active,no_triangles);

  int unique = 0;
  for(int i = 0; i <256; i++){
    for(int j = 0; j <no_threads;j++) {
        
      unique_patterns[i]+= per_thread_patterns[j][i];

    }
    if (unique_patterns[i] != 0) {
      unique++;
      printf("u[%d]=%lu  ", i, unique_patterns[i]);
    }
  }
  printf("\nFOund %d unique patterns\n", unique);

//  e->cancel_compute();

//  printf("All done\n");
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << "Finish motif counting. Running time : " << diff.count() << " s\n";

//  fsync(clique_fd);
}









