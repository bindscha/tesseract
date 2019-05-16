#include "tesseract.hpp"
//Engine<Embedding<uint32_t>>

bool _mmap = false;
bool indexed = false;

int c;
std::string tmp;
size_t b_size = 0,initial_chunk = 0;
int set_K = 3;
int algo = 0;
int w_id = 0;
int no_workers = 1;

static void usage(void) {
  printf("Usage: ./tesseract -a [algorithm 0:k-clique;1:motif] -f <graph file> -d <degree file> -n <nb_nodes> -t <nb_threads> [ -u (run update_algo) -b <batch_size> -c <preload_initial_edges-0 default> ]\n");

  _exit(-1);
}

//
// BEGIN NEW API
//

void init(size_t worker_id, size_t num_workers) {
    printf("Initialized Tesseract worker %lu (out of %lu)\n", worker_id, num_workers);
}

void start() {
    printf("Started execution!\n");
}

void stop() {
    printf("Stopped execution!\n");
}

void edge_new(uint32_t src, uint32_t dst, uint32_t ts) {
    printf("Received new edge %u -> %u (ts=%u)\n", src, dst, ts);
}

void edge_del(uint32_t src, uint32_t dst, uint32_t ts) {
    printf("Received del edge %u -> %u (ts=%u)\n", src, dst, ts);
}

//
// END NEW API
//

void init_tesseract( int _w_id =0,int _no_workers=1,bool _mmap=0){
  printf("Running on graph %s (%lu nodes) with %lu threads\n", input_file, NB_NODES, no_threads);

//  e = new Engine<edge_full>();
  no_active_next = 0;
//  e->init();
  init(_mmap);
  init_thread_d_gen<Embedding<uint32_t>>(clique_fd);


  printf("[STAT] no_active pre-compute: %lu\n", no_active);

}

void load_graph_and_init(char* i_fname, char* o_fname,bool do_up,size_t n_nodes,int n_threads,int algo_id,size_t bs,size_t _initial_chunk,int _w_id, int _no_workers){
  input_file = i_fname;
  degree_file = o_fname;
  do_updates = do_up;
  NB_NODES= n_nodes;
  no_threads = n_threads;
  b_size = bs;
  initial_chunk = _initial_chunk;
  algo = algo_id;

  init_tesseract(_w_id,_no_workers);
}
void load_graph_and_init(char* i_fname, char* o_fname,bool do_up,size_t n_nodes,int n_threads,int algo_id,size_t bs,size_t _initial_chunk,int _w_id, int _no_workers,bool _mmap,char* _update_file){
  input_file = i_fname;
  degree_file = o_fname;
  do_updates = do_up;
  NB_NODES= n_nodes;
  no_threads = n_threads;

  b_size = bs;
  initial_chunk = _initial_chunk;
  algo = algo_id;
  update_file = _update_file;
  init_tesseract(_w_id,_no_workers,_mmap);
}
void start_exec(){

  //  _no_workers = no_threads;
  switch(algo){
    case 0: {
      assert(SYM == 1);
      printf("Looking for %d-cliques...\n", K);
      Engine <Embedding<uint32_t>, CliqueFindE> *e = new Engine <Embedding<uint32_t>, CliqueFindE>(b_size,
                                                                                                   initial_chunk,
                                                                                                   new CliqueFindE(),
                                                                                                   w_id, no_workers);
      if (!do_updates)
        e->algo->activate_nodes();
      printf("Starting computation...\n");
      auto start = std::chrono::high_resolution_clock::now();

      e->execute_app();

      printf("Finished computation!\n");
      printf("[STAT] no_active post-compute: %lu\n", no_active);
      printf("Results:\n");
      printf("  %d-cliques found: %lu\n", K, no_triangles);

      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> diff = end - start;
      std::cout << "  Runtime: " << diff.count() << " seconds\n";

      e->algo->output();
//      e->algo = new CliqueFindE();
      //  e->algo = new CliqueIndex();
    }
      break;
    case 1: {
      assert(SYM == 0);
      Engine <Embedding<uint32_t>, MotifCountingE> *e = new Engine <Embedding<uint32_t>, MotifCountingE>(b_size,
                                                                                                         initial_chunk,
                                                                                                         new MotifCountingE(),
                                                                                                         w_id,
                                                                                                          no_workers);
      printf("Looking for %d-motifs...\n", K);
      if (!do_updates)
        e->algo->activate_nodes();
      printf("Starting computation...\n");
      auto start = std::chrono::high_resolution_clock::now();

      e->execute_app();

      printf("Finished computation!\n");
      printf("[STAT] no_active post-compute: %lu\n", no_active);
      printf("Results:\n");
      printf("  %d-cliques found: %lu\n", K, no_triangles);

      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> diff = end - start;
      std::cout << "  Runtime: " << diff.count() << " seconds\n";

      e->algo->output();
    }
      break;
    case 2: {
      assert(SYM == 1);
      Engine <Embedding<uint32_t>, ColorCliqueE> *e = new Engine <Embedding<uint32_t>, ColorCliqueE>(b_size,
                                                                                                     initial_chunk,
                                                                                                     new ColorCliqueE(),
                                                                                                     w_id, no_workers);
//      e->algo = new MotifCountingE();

      printf("Looking for %d-color cliques...\n", K);
      if (!do_updates)
        e->algo->activate_nodes();
      printf("Starting computation...\n");
      auto start = std::chrono::high_resolution_clock::now();

      e->execute_app();

      printf("Finished computation!\n");
      printf("[STAT] no_active post-compute: %lu\n", no_active);
      printf("Results:\n");
      printf("  %d-cliques found: %lu\n", K, no_triangles);

      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> diff = end - start;
      std::cout << "  Runtime: " << diff.count() << " seconds\n";

      e->algo->output();
    }
      break;
    default:
      printf("Need to define an algo \n");
      exit(1);
  }


}

inline void print_progress() {
  size_t found = 0;
  for(int i =0 ; i< no_threads;i++){
    found += per_thread_data[i];
  }
  printf("Found %lu \n",found);
}
int main(int argc, char** argv)  {

  while ((c = getopt(argc, argv, "f:d:vein:t:b:u:k:c:a:w:W:m")) != -1) {
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
      case 'i':
        indexed = true; //exp_mode[atoi(optarg)];
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
        b_size = stoul(tmp);
        printf("Update batch size %lu\n",b_size);
        break;
      case 'u':
        do_updates = 1;
        update_file = optarg;
        EXPLORE_MODE =MIDDLE;
        printf("Running updates \n");
        break;
      case 'c':
        tmp = std::string(optarg);
        initial_chunk = stoul(tmp); //preload initial_chunk edges before starting to apply updates
        printf("Preloading %lu edges\n ",initial_chunk);
        break;
      case 'k':
//              K = atoi(optarg);
        break;
      case 'a': //Algo
        algo = atoi(optarg);
        break;
      case 'm':
        _mmap = true;
        printf("MMAP-ing input and output\n");
        break;

//      case 'w':
//        _w_id = atoi(optarg);
//        break;
//      case 'W':
//        _no_workers=atoi(optarg);
//        break;
    }

  }

  init_tesseract(0, 1, _mmap);


  start_exec();

  return 0;
}