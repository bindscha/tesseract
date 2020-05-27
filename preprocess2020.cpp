//
// Created by Jasmina Malicevic on 2020-04-19.
// Sorts an input edge array by source and creates reverse edges. The output is a sorted EDGE array and the offsets of the corresponding vertices
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <locale.h>
#include <sys/mman.h>
#include <iostream>
#include <assert.h>
#include <unordered_set>
#include <thread>
#include <vector>
#define VID_SIZE uint32_t

struct edge{
    VID_SIZE src;
    VID_SIZE dst;
//    VID_SIZE w;
};

struct edge_adj{
    VID_SIZE src;
    VID_SIZE dst;
};
const int NO_THREADS= 55;
void readVectorsRev(size_t NB_EDGES, edge* input, std::vector<std::vector<VID_SIZE>>& edges, size_t nb_nodes, int tid, bool dir){



    for(uint64_t idx = 0; idx < NB_EDGES; idx++){
        edge e  = input[idx];
        assert(e.src< nb_nodes);
        assert(e.dst < nb_nodes);
        if (e.dst % NO_THREADS == tid && e.dst != e.src){
            bool found = false;
            for(size_t i =0 ; i < edges[e.dst].size(); i++){
                if(edges[e.dst][i] == e.src){
                    found = true;
                    break;
                }
            }
//            for(auto x: edges[e.dst]){
//                if(x == e.src){
//                    found =true;
//                    break;
//                }
//            }
            if(!found)
                edges[e.dst].emplace_back(e.src);
        }


//        if(e.dst % NO_THREADS == tid){
//            edges_rev[e.dst].insert(e.src);
//        }


    }
//    std::cout << processed  << " (" <<dups<<")" <<"\n";
}
void readVectors(size_t NB_EDGES, edge* input, std::vector<std::vector<VID_SIZE>>& edges, size_t nb_nodes, int tid, bool dir){



    for(uint64_t idx = 0; idx < NB_EDGES; idx++){
        edge e  = input[idx];
        assert(e.src< nb_nodes);
        assert(e.dst < nb_nodes);
        if (e.src % NO_THREADS == tid && e.src != e.dst){
            bool found = false;
            for(size_t i =0 ; i < edges[e.src].size(); i++){
                if(edges[e.src][i] == e.dst){
                    found = true;
                    break;
                }
            }
//            for(auto x: edges[e.src]){
//                if(x == e.dst){
//                    found =true;
//                    break;
//                }
//            }
            if(!found)
                edges[e.src].emplace_back(e.dst);
        }


//        if(e.dst % NO_THREADS == tid){
//            edges_rev[e.dst].insert(e.src);
//        }


    }
//    std::cout << processed  << " (" <<dups<<")" <<"\n";
}

void readIn(size_t NB_EDGES, edge* input, std::unordered_set<VID_SIZE>* edges, size_t nb_nodes, int tid, bool dir){
    uint64_t num = NB_EDGES / 56;

    uint64_t idx = tid * num;
    uint64_t stop = idx + num;
    if(tid == 55){
        stop = NB_EDGES;
    }
    for(;idx < stop; idx++){
        if(idx > 0){
            assert(input[idx-1].src <= input[idx].src);
        }
    }

//    for(uint64_t idx = 0; idx < NB_EDGES; idx++){
//        edge e  = input[idx];
//        assert(e.src< nb_nodes);
//        assert(e.dst < nb_nodes);
//        if (e.src % NO_THREADS == tid && e.src != e.dst)
//            edges[e.src].insert(e.dst);
////        if(e.dst % NO_THREADS == tid){
////            edges_rev[e.dst].insert(e.src);
////        }
//
//
//    }
//    std::cout << processed  << " (" <<dups<<")" <<"\n";
}

void mergeLists(size_t nb_nodes, std::unordered_set<VID_SIZE>*edges, std::unordered_set<VID_SIZE>*edges_rev, int tid){
    VID_SIZE start = tid * nb_nodes/NO_THREADS;
    VID_SIZE end = start + nb_nodes / NO_THREADS;
    if(tid == NO_THREADS) end = nb_nodes;

    for(;start <end;start++){
        edges[start].insert(edges_rev[start].begin(), edges_rev[start].end());
    }
}

void readOut(size_t NB_EDGES, edge* input, std::unordered_set<VID_SIZE>* edges, size_t nb_nodes, int tid, bool dir){
//    std::cout << " Thread " << tid << "started  set \n";

    for(uint64_t idx = 0; idx < NB_EDGES; idx++){
        edge e  = input[idx];


            if (e.dst % NO_THREADS != tid || e.src == e.dst) continue;

            edges[e.dst].insert(e.src);
    }
//    std::cout << " Thread " << tid << "done\n";
}
void setOffsets(size_t nb_nodes, edge_adj* out_edge_array, std::vector<std::vector<VID_SIZE>>& edges, int i,size_t * offsets, size_t* offset_out, size_t NEW_EDGES){
    VID_SIZE start = i * (nb_nodes/NO_THREADS);
    VID_SIZE end = start +(nb_nodes / NO_THREADS);
    if(i == NO_THREADS -1) end = nb_nodes;

//    std::cout <<"Ti d " << i << " ["<<start << " - " << end << "]\n";
    uint64_t curr = 0;
    for(;start < end; start++) {
        assert(start < nb_nodes);
        curr = 0;
        offset_out[start] = offsets[start];
        for (auto dst: edges[start]) {

            assert(offsets[start] + curr <NEW_EDGES);
            assert(start != dst);
            out_edge_array[offsets[start]+ curr].src = start;
            out_edge_array[offsets[start] + curr].dst = dst;
            curr++;

        }
    }
}
int main(int argc, char* argv[]){
    size_t nb_nodes = atol(argv[1]);

    char* inFile = argv[2];
        char* outFile = argv[3];
        char* offsetFile = argv[4];
        struct stat sb;
        int c = open(inFile,  O_RDWR| O_CREAT |O_NOATIME | O_DIRECT,0600);
        fstat(c, &sb);
        uint64_t NB_EDGES = sb.st_size / sizeof(edge);
        std::cout <<"NB NODES " << nb_nodes << std::endl;

        edge* edge_array = (edge*) mmap(NULL, NB_EDGES * sizeof(edge), PROT_READ , MAP_SHARED, c, 0);

    //    std::unordered_set<VID_SIZE>* edges = new std::unordered_set<VID_SIZE>[nb_nodes];
    //    std::unordered_set<VID_SIZE>* edges_rev = new std::unordered_set<VID_SIZE>[nb_nodes];

        std::vector<std::thread> threads;
        std::vector<std::vector<VID_SIZE>> edges;
        edges.reserve(nb_nodes); //)= (std::vector<VID_SIZE>*)calloc(nb_nodes, sizeof(std::vector<VID_SIZE>*));
        for(size_t i = 0; i  < nb_nodes;i++){
            edges[i].reserve(4);
        }
//    for(int i = 0; i < NO_THREADS; i++){
//        threads.push_back(std::thread(ge`tDegree, NB_EDGES, std::ref(edge_array), std::ref(edges),nb_nodes, i, true));
//    }
//
//    for(int i = 0; i < NO_THREADS; i++){
//        threads[i].join();
//    }
    for(int i = 0; i < NO_THREADS; i++){
        threads.push_back(std::thread(readVectors,NB_EDGES, std::ref(edge_array), std::ref(edges),nb_nodes, i, true));
    }

    for(int i = 0; i < NO_THREADS; i++){
        threads[i].join();
    }

//    for(int i = 0; i < NO_THREADS; i++){
//        threads[i] = std::thread(readVectorsRev,NB_EDGES, std::ref(edge_array), std::ref(edges),nb_nodes, i, true);
//    }
//
//    for(int i = 0; i < NO_THREADS; i++){
//        threads[i].join();
//    }
    std::cout << "Loaded file \n";

//    for(int i = 0; i < NO_THREADS; i++){
//        threads[i] = std::thread(mergeLists, nb_nodes, std::ref(edges), std::ref(edges_rev),i);
//    }
//    for(int i = 0; i < NO_THREADS; i++){
//        threads[i] = std::thread(readOut, NB_EDGES, std::ref(edge_array), std::ref(edges),nb_nodes, i, false);
//    }
//
//    for(int i = 0; i < NO_THREADS; i++){
//        threads[i].join();
//    }


    size_t* offsets = (size_t*) calloc(nb_nodes, sizeof(size_t));
    offsets[0] = 0;
    size_t NEW_EDGES = edges[0].size();
    for(VID_SIZE i = 1 ;i < nb_nodes; i++){
//        NEW_EDGES += edges[i].size();
        offsets[i] = NEW_EDGES;
        NEW_EDGES += edges[i].size();
//        offsets[i] += offsets[i-1] + edges[i - 1].size();

//        assert(offsets[i]  + edges[i].size() < NEW_EDGES+edges[i].size());
    }
////    offsets[]
////std::cout<<  " edges[60458804] " << edges[60458804].size() << " off" << offsets[60458804] <<"\n";
    std::cout << "New edges " << NEW_EDGES << " \n" ; //edges[61578412] " << edges[61578412].size() << " off" << offsets[61578412]<< " LAST: "<< edges[nb_nodes -1].size() <<" " << offsets[nb_nodes - 1] << "\n";
//
    int offset_fd = open(offsetFile, O_RDWR | O_CREAT |O_NOATIME |O_DIRECT, 0600);
    fallocate(offset_fd,0,0, nb_nodes * sizeof(size_t));
    size_t* offset_out = (size_t* )mmap(NULL, nb_nodes * sizeof(size_t), PROT_READ| PROT_WRITE, MAP_SHARED, offset_fd, 0);

    int out_fd = open(outFile, O_RDWR| O_CREAT |O_NOATIME | O_DIRECT,0600);


    if(fallocate(out_fd,0,0, NEW_EDGES * sizeof(edge_adj)) == -1){
        std::cout <<"error in allocation \n";
    }
    edge_adj* out_edge_array = (edge_adj*) mmap(NULL, NEW_EDGES  * sizeof(edge_adj), PROT_READ | PROT_WRITE, MAP_SHARED, out_fd, 0);

    if(out_edge_array == MAP_FAILED){
        std::cout <<"error in mmap \n";
    }
    for(int i = 0; i < NO_THREADS; i++){
        threads[i] = std::thread(setOffsets, nb_nodes,std::ref(out_edge_array), std::ref(edges), i, std::ref(offsets), std::ref(offset_out), NEW_EDGES);

    }
    for(int i = 0; i < NO_THREADS; i++){
        threads[i].join();
    }
////    std::cout<< "*** " << offset_out[nb_nodes - 1] << " vs " << offsets[nb_nodes-1] << "\n";
////    std::cout<<  " edges[60458804] " << edges[60458804].size() << " off" << offset_out[60458804] <<"\n";
//
    fsync(out_fd);
    munmap(out_edge_array, NEW_EDGES* sizeof(edge_adj));
    close(out_fd);
    fsync(offset_fd);
    munmap(offset_out, nb_nodes* sizeof(size_t));
    close(offset_fd);
//
//    close(c);





}