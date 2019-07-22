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
    bool del = false;
    while ((c = getopt(argc, argv, "f:d:n:t:b:uc:a:r")) != -1) {
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

                printf("Running updates \n");
                break;
            case 'c':
                tmp = std::string(optarg);
                initial_chunk = stoul(tmp); //preload initial_chunk updates before starting to apply updates
                printf("Preloading %lu updates\n ", initial_chunk);
                break;
            case 'r': //UPDATES ARE REMOVALS
                updateType = GraphUpdateType::EdgeDel;
                printf("Deleting edges \n");
                del = true;
                break;
            case 'a': //Algo
                configuration->algorithm_id = atoi(optarg);
                break;
//            case 'm':
//                _mmap = true;
//                printf("MMAP-ing input and output\n");
//                break;
        }
    }

    configuration->worker_id = 0;
    configuration->num_workers = 1;

    //For single machine case with a file as input:
    setGraphInputFiles(graphInput);
    if(do_updates)
        init_update_buf(b_size, NB_EDGES, NB_NODES, 1, del? NB_EDGES :initial_chunk);
    if(initial_chunk == 0)
     init(configuration);

    std::thread engine_th;
    if(initial_chunk ==0)
       engine_th = std::thread(start); //start  Engine

    if(do_updates){
        if(initial_chunk != 0 ){
            preloadChunk(del?NB_EDGES:initial_chunk,configuration); // If deletions, load the entire graph
            init(configuration);
            engine_th = std::thread(start);
            printf("Done preloading\n");

        }
        GraphUpdate* update_stream = new GraphUpdate[b_size];
        size_t no_batches = 1;
        size_t total_added = 0;
//        for(size_t j = NB_EDGES -1; total_added < initial_chunk && j >= 0; ){ //DELETE EDGES AT END
//            size_t items_added = 0;
//            size_t i = 0;
//            for(; items_added < b_size && j >= 0; j--){//} i++){
//                if(edges_full[j ].src > edges_full[j].dst)continue;
////                if(edges_full[j ].src != 0)continue;
//                update_stream[items_added].src = edges_full[j].src;
//                update_stream[items_added].dst = edges_full[j].dst;
//                update_stream[items_added].ts = no_batches;
//                update_stream[items_added].tpe = del?EdgeDel: EdgeAdd;
//                items_added++;
//                total_added++;
//                if(total_added == initial_chunk) break;
//            }
////            j-=i;
////            total_added += items_added;
//            printf("Processed %lu of %lu (%lu) ^^^\n ", items_added, j, NB_EDGES);
//            no_batches++;
//            batch_new(update_stream, items_added);
//            if(total_added == initial_chunk) break;
//        }
        for(size_t j = del?0 : initial_chunk; del? (j < initial_chunk|| j<NB_EDGES) : j< NB_EDGES;){ //Deletes/Adds first INITIAL_CHUNK_EDGES
            if(j + b_size > NB_EDGES) b_size = NB_EDGES - j;

            volatile size_t items_added = 0;
            for( ;items_added < b_size && j < NB_EDGES ; j++ ) {
                if(edges_full[j].src > edges_full[j ].dst) continue;
//                if(edges_full[j].src != 0 )continue;
                update_stream[items_added].src = edges_full[j].src;
                update_stream[items_added].dst = edges_full[j].dst;
                update_stream[items_added].ts = no_batches;
                update_stream[items_added].tpe = del?EdgeDel: EdgeAdd;
                items_added++;
                total_added++;
                if(del && total_added == initial_chunk) break;
            }

            printf("Processed %lu of %lu (%lu) ^^^\n ", items_added, j, NB_EDGES);
            no_batches++;
            batch_new(update_stream, items_added);
            if(total_added == initial_chunk) break;
        }
        //Start update engine
    }
    stop();
    engine_th.join();







//    init_tesseract(0, 1, _mmap);


//    start_exec();

    return 0;
}
