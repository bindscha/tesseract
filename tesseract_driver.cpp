//
// Created by jasmi on 7/1/2019.
//

#include "libtesseract.h"
#include "graph.hpp"


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
                initial_chunk = stoul(tmp); //preload initial_chunk edges before starting to apply updates
                printf("Preloading %lu edges\n ", initial_chunk);
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

    //For single machine case with a file as input:
    setGraphInputFiles(graphInput);
    init(configuration);

    if(do_updates){
        if(initial_chunk != 0 ){
            // Fill initial chunk
        }
        //Generate batch of batch_size
        //Start update engine
    }
    start(); //start  Engine





//    init_tesseract(0, 1, _mmap);


//    start_exec();

    return 0;
}
