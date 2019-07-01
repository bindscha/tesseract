//#define SYM 1
#include "engine_emb.hpp"
//#include "engine_index.hpp"

size_t discarded = 0;

//#define K 2
/*
 * Algo data
 *
 */

//Pattern<uint32_t, edge_full>* cliques_found; //this is mmaped to a file; TODO it could be done a bit better



Engine<Embedding<uint32_t>>*e;
//Engine<edge_full>*e;

void activate_nodes(){

//  if(EXPLORE_MODE== MIDDLE){
//  for(uint32_t i = 0; i < NB_NODES; i++) {
//    if(e->algo->prefilter(i))
//      for (size_t idx = 0; idx < degree[i]; idx++) {
//        if (e->algo->prefilter(edges[adj_offsets[i] + idx].dst) && edges[adj_offsets[i] + idx].dst > i)
//          active[no_active++] = adj_offsets[i] + idx;
//      }
//  }
//    return;

    for (uint32_t i = 0; i < NB_NODES; i++) {
        if (e->algo->prefilter(i))//should_be_active(i))
            for (size_t idx = 0; idx < degree[i]; idx++) {
                if (e->algo->prefilter(edges_full[adj_offsets[i] + idx].dst) && edges_full[adj_offsets[i] + idx].dst >
                                                                                i) //should_be_active(edges_full[adj_offsets[i] + idx].dst) && edges_full[adj_offsets[i] + idx].dst > i)
                    active[no_active++] = adj_offsets[i] + idx;
            }
    }
}


int main(int argc, char **argv) {
    bool indexed = false;
    SYMMETRIC = true;
    int c;
    std::string tmp;
    size_t b_size = 0,initial_chunk = 0;
    int set_K = 3;
    while ((c = getopt(argc, argv, "f:d:vmein:t:b:uk:c:")) != -1) {
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
              break;
            case 'u':
              do_updates = 1;
            break;
            case 'c':
              tmp = std::string(optarg);
              initial_chunk = stoul(tmp); //preload initial_chunk edges before starting to apply updates
            break;
            case 'k':
//              K = atoi(optarg);
              break;
        }

    }

  printf("Running on graph %s (%lu nodes) with %lu threads\n", input_file, NB_NODES, no_threads);

  e = new Engine<Embedding<uint32_t>>(b_size,initial_chunk);
//  e->K = set_K;
  printf("Looking for %d-cliques...\n", K);
//  e = new Engine<edge_full>();
  no_active_next = 0;

//  e->batch_size=b_size;
  printf("Index: on\n");
  printf("Index: off\n");
  e->algo = new CliqueFindE();
  printf("Updates %d\n",do_updates);
//  e->algo = new CliqueIndex();
//    DIRECTION = FWD;
//
//    init();
//    init_thread_d_gen<Embedding<uint32_t>>(clique_fd);

    if(!do_updates)
    e->init_active();//&(e->algo->activate_nodes));
    printf("[STAT] no_active pre-compute: %lu\n", no_active);

    printf("Starting computation...\n");
    auto start = std::chrono::high_resolution_clock::now();

    e->execute_app();

    printf("Finished computation!\n");
    printf("[STAT] no_active post-compute: %lu\n", no_active);
    printf("Results:\n");
    printf("  %d-cliques found: %lu\n", K, no_triangles);

//    e->cancel_compute();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "  Runtime: " << diff.count() << " seconds\n";
    //  fsync(clique_fd);
}









