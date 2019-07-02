//
// Created by jasmi on 7/1/2019.
//

#include "libtesseract.h"
#include "graph.hpp"
#include <thread>

bool _mmap = false;
bool indexed = false;
int c;
std::string tmp;
size_t b_size = 0,initial_chunk = 0;
int main(int argc, char** argv)  {
    Configuration* configuration = new Configuration();
    GraphInputFiles* graphInput = new GraphInputFiles();
    int no_threads;
    while ((c = getopt(argc, argv, "f:d:vein:t:b:u:k:c:a:w:W:m")) != -1) {
        switch (c) {
            case 'f':
                graphInput->input_file = optarg;
                break;
            case 'd':
                graphInput->degree_file = optarg;
                break;
            case 'i':
                indexed = true; //exp_mode[atoi(optarg)];
                break;
            case 'n':
                tmp = std::string(optarg);
                graphInput->nb_nodes = stoul(tmp);
//                NB_NODES = stoul(tmp);
                break;
            case 't':
                tmp = std::string(optarg);
                configuration->no_threads = stoul(tmp);
                break;
            case 'b':
                tmp = std::string(optarg);
                b_size = stoul(tmp);
                printf("Update batch size %lu\n", b_size);
                break;
            case 'u':
                do_updates = 1;
                update_file = optarg;

                printf("Running updates \n");
                break;
            case 'c':
                tmp = std::string(optarg);
                initial_chunk = stoul(tmp); //preload initial_chunk updates before starting to apply updates
                printf("Preloading %lu updates\n ", initial_chunk);
                break;
            case 'k':
//              K = atoi(optarg);
                break;
            case 'a': //Algo
                configuration->algorithm_id = atoi(optarg);
                break;
            case 'm':
                _mmap = true;
                printf("MMAP-ing input and output\n");
                break;
        }
    }

    configuration->worker_id = 0;
    configuration->num_workers = 1;
    if(do_updates)
        init_update_buf(b_size, NB_EDGES, NB_NODES,  initial_chunk);
    //For single machine case with a file as input:
    setGraphInputFiles(graphInput);
    init(configuration);


    std::thread engine_th(start); //start  Engine

    if(do_updates){
        if(initial_chunk != 0 ){
            preloadChunk(initial_chunk);
        }
        GraphUpdate* update_stream = new GraphUpdate[b_size];
        size_t no_batches = 0;
        for(size_t j = initial_chunk; j < NB_EDGES; j+= b_size){
            for(size_t i = 0; i < b_size; i++) {
                update_stream[i].src = edges_full[i].src;
                update_stream[i].dst = edges_full[i].dst;
                update_stream[i].ts = no_batches;
                update_stream[i].tpe = EdgeAdd;
            }
            batch_new(update_stream, b_size);
        }
        //Start update engine
    }
    engine_th.join();






//    init_tesseract(0, 1, _mmap);


//    start_exec();

    return 0;
}
