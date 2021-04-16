# Tesseract: Fast, Scalable Graph Pattern Mining on Evolving Graphs

Currently Mongo has been disabled and everything runs in memory of a single machine. 

##Input format
Tesseract takes a graph in the form of an adjacency list with two input files:

- List of edges sorted by source - edgefile
- File containing offsets in the edgefile. For each vertex ID (Starting with 0), there is an offset pointing to its neighbours - offsetFile

###Reading the input in code
TODO describer where is input read and the caveats.

##Compilation
Create cmake-build directory and `cd` into ti. Run `cmake ../; make`.

##Running
`libtesseract` will run the code with the following parameters:
- `-f` Path to edgefile
- `-d` Path to offsets file
- `-n` Number of vertices in the graph
- `-a` Algorithm: 0 - cliques, 1 - motif count, 3 - GKS (graph keyword serach)
- `-t` Number of threads
- `-u` If set, tesseract will run the incremental version of the algorithm according to the following parameters:
    - `-b` Batch size
    - `-c` Number of edges to preload before adding the remaining edges. If not set, the graph will be built entirely form scratch
    

###Preloading edges for incremental computation
We can preload a part of the graph and then load only X edges as updates. 
The `updateBuffers.hpp` function had a shuffle function which is commented out. This process takes in practice a lot of time. 
For our experiments we generated update files, that are randomized edges and loaded that. 

You can comment these lines out to preload the edges as they arrive in the input file. 

###Code structure
The entry point is `tesserect_driver` which calls `libtesseract.cpp`. The later has the interface classes used also by the scala algorithms to execute Tesseract.

`engine_one.hpp` is where the main logic is happening. There are two main classes of drivers: 
- Static - to run the exploration on static graphs
- Dynamic - to run the exploration on dynamic graphs

We differentiate between **symmetric** and **non symmetric** algorithms. Symmetric algorithms are for example cliques where we need all to all connectivity. 
These algorithms can be optimized by exploring only the neighbours who have a higher vertex Id than the last added vertex. 

Non symmetric algorithms, like Motif Count and GKS, are general algorithms for which we cannot exclude other neighbours and we need to do a loop over all the neighbours.

#Code caveats
The current state of the code is not final and requires some manual tweaking to try out different scenarios.
A fix for these is in progress. 

#Motif counting and GKS

These algorithms are very similar but after some changes to the code, the compiler was not optimizing the functions as expected so we added code for those in `engine_one.hpp`.
- If you are running Motif counting , uncomment the `#define MOTIF 1` statement at the beginning of the file. 
- If you are running GKS, uncomment the places with `uncomment when running GKS`


##Setting the K in algorithms
Right now the size of the pattern (clique, motif etc.) is set manually in `common_include.hpp`

##Disabling/Enabling caching
The embedding cache can be enabled in `engine_one.hpp` by setting `e_cache_enabled` to `false` in **two** places. 
By default it is disabled.

##Static vs dynamic execution

To run static code:

File: `libtesseract.cpp`

Function: `init`, line 50. - Comment out all the :

`if (input)... else` code.

- To speed up the static case, we comment out `#define EDGE_TIMESTAMPS` in `embedding.hpp`. Right now this is also needed for correctness.
  This simulates the case where edges are not assigned timestamps and we process less data (as the static systems would)
  
##Code commenting since the compiler does not efficiently optimize the templated function

- When we last tested the code ,we had problems with the optimizer as it would not optimize classes properly and we got a huge performance penalty.

Therefore, to achieve the best running time, we had to explicitly comment out all but the algorithm we want to run.

- To get the best numbers for the static algorithm, we commented out the calls to `DynamicDriver`. And to get the best performance of the `DynamicDriver`, 
commenting out every reference to `Static driver` was needed. This was all done in `libtesseract.cpp` Function `init` 

#Computation on preloaded part of the graph
If you uncomment the calls to the static engine in the if branch when doing updates, there will be computation performed on the preloaded part of the graph.
For updates however, especially for big graphs, we recommend leaving this commented out. 

#Other tips
On big machines ,it is beneficial to manage the dataplacement on NUMA nodes and pin the threads to cores with taskset.  
For example, running 4 Cliques on Mico (which is a small graph) on a big machine with 1TB of RAM and 56 threads with

` numactl --interleave=all taskset -c 0-55 ./libtesseract -f /media/nvme/osdi_inputs/mico-sorted -d /media/nvme/osdi_inputs/mico-sorted_offsets -n 100000 -a 0 -t 56` took 11s.
Without `taskset` it took >300s.

