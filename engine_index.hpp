#include "common_include.hpp"

#include "barrier.hpp"
#include "algo_api.hpp"


#include "utils.hpp"
#include "graph.hpp"

#include "clique_vector.hpp"

#include "Pattern.hpp"
#include "graph_cache.hpp"
#include "embedding.hpp"
#include "canonic_checks.hpp"

#include "algorithm.hpp"
#include "motif_counting.hpp"
#include "triangle_c.hpp"
#include "cliques_ooc.hpp"
#include "cliques_index.hpp"
#include "thread_data.hpp"
#include <set>
#include <unordered_set>
#include <thread>
#include <mutex>



size_t curr_cliques = 0;

int phase = 0;

int step = 0;


uint64_t no_triangles = 0;

enum exp_mode {
    VERTEX, EDGE, MIDDLE
};

exp_mode EXPLORE_MODE = VERTEX;
int max = K;

void test_c(int c) {

}


void printPattern(Pattern<uint32_t, edge_full> *set, uint32_t step, uint32_t v_id) {

    printf("\n");
    for (int i = 0; i < step; i++) {
        if (i != 0)
            printf("(%u)%u ", v_id, set->vertices_in_pattern[i]);
        else {
            uint32_t src = set->vertices_in_pattern[i] & mask;
            if (src == 0)src = set->vertices_in_pattern[i];
            else src = set->vertices_in_pattern[i] - mask;
            printf("%u ", src);//set->vertices_in_pattern[i]);
        }
    }
    printf("(%u)%u\n", v_id, set->vertices_in_pattern[step]);

}

template<typename T>
class Engine {

private:

    std::thread **threads;

    GraphCache<Embedding<uint32_t>> cache;

    std::unordered_map<uint32_t, std::set<uint32_t>> *index;
    std::mutex mtx;

    bool iddfs = false;
    bool use_index = true;

public:
    uint64_t batch_size = 10000;
    Algorithm<Pattern<uint32_t, T>> *algo;

    Engine() {
        threads = (std::thread **) calloc(no_threads - 1, sizeof(void *));
        index = new std::unordered_map<uint32_t, std::set<uint32_t>>[NB_NODES];
    }

    ~Engine() {
        delete[] threads;
        delete[] index;
    }

    inline void init_active(void(*func)()) {
        no_active = 0;
        func();
    }

    inline void explore_phase1(Embedding<uint32_t> *embedding, uint32_t step, int tid) {
        explore_v(embedding, step, tid);
    }

    inline void explore_phase2(uint32_t vid, int tid) {
        if (iddfs) {
            explore_phase2_simple(vid, tid);
        } else {
            if (K == 4) {
                explore_phase2_optimized_4k(vid, tid);
            } else if (K == 5) {
                explore_phase2_optimized_5k(vid, tid);
            } else {
                explore_phase2_simple(vid, tid);
            }
        }
    }

    inline void explore_phase2_simple(uint32_t vid, int tid) {
        if (degree[vid] >= K && cache[vid].valid()) {
            const uint32_t size = cache[vid].size();
            for (uint32_t i = 0; i < size; ++i) {
                Embedding<uint32_t> *embedding = &(cache[vid][i]);
                explore_v(embedding, embedding->no_vertices(), tid);
            }
        }
    }

    inline void explore_phase2_optimized_4k(uint32_t vid, int tid) {
        if (use_index) {
            // 4K(a, b, c, d) := 3K(a, b, c), 3K(a, b, d), 3K(a, c, d)
            if (degree[vid] >= K - 1 && index[vid].size() > 1) {
                for (const auto& pair : index[vid]) { // Go through (x, y, z) with x = a
                    uint32_t b = pair.first; // Get y = b
                    for (uint32_t c : index[vid][b]) { // Go through (x, y, z) with x = a and y = b
                        if (index[vid].count(c)) { // If there is z = c s.t. (xx, yy, zz) with xx = a and yy = c
                            for (uint32_t d : index[vid][b]) { // Go through (a, b, z)
                                if (c != d && index[vid][c].count(d)) { // If there is another z = d != c, then 4K found!
                                    ++per_thread_data[tid];
                                }
                            }
                        }
                    }
                }
            }
        } else {
            // Using cache here is a bad idea as the search is O(n^3)
            // I can write it in O(n^2) with some work, but it will still suck
            /*if (degree[vid] >= K && cache[vid].valid()) {
                const uint32_t size = cache[vid].size();
                for (uint32_t i = 0; i < size; ++i) {
                    Pattern<uint32_t, T>& x = cache[vid][i];
                    for (uint32_t j = i + 1; j < size; ++j) {
                        Pattern<uint32_t, T>& y = cache[vid][j];
                        if (x.vertices_in_pattern[1] == y.vertices_in_pattern[1]) {
                            for (uint32_t k = 0; k < size; ++k) {
                                if (k == i || k == j) {
                                    continue;
                                }
                                Pattern<uint32_t, T>& z = cache[vid][k];
                                if (x.vertices_in_pattern[2] == z.vertices_in_pattern[1] && y.vertices_in_pattern[2] == z.vertices_in_pattern[2]) {
                                    ++per_thread_data[tid];
                                }
                            }
                        }
                    }
                }
            }*/
        }
    }

    inline void explore_phase2_optimized_5k(uint32_t vid, int tid) {
        // XXX: todo
    }

    inline void explore_v(Embedding<uint32_t> *embedding, uint32_t step, int tid) {
        uint32_t v_id = embedding->last();
        uint32_t dst;

        FOREACH_EDGE_FWD(v_id, dst)
            if (degree[dst] < step) { //if(!algo->prefilter(dst)){ // XXX: prefilter should receive embedding or step!!
                continue;
            }

            embedding->append(dst);

            const bool filter = embedding->no_edges() == ((step+1)* (step))/2; //algo->filter(dst, embedding, step);
            if (filter) {
                if (step < K - 2) {
                    explore_v(embedding, step + 1, tid);
                } else if (step == K - 2) {
                    //algo->process(embedding, step, tid);

                    /*if (use_index) {
                        mtx.lock();
                        index[(*embedding)[0]][(*embedding)[1]].insert((*embedding)[2]);
                        mtx.unlock();
                    } else {
                        cache.append_to_vertex((*embedding)[0], *embedding);
                    }*/
                    ++per_thread_data[tid];
                } else {
                    //algo->process(embedding, step, tid);
                    ++per_thread_data[tid];
                }
            }
            embedding->pop();
        ENDFOR
    }

    void compute(void *c) {
        int tid = (long) c;
        assert(tid < no_threads);

        if (tid == 0 && !use_index) {
            cache.resize(NB_NODES);
        }

        uint64_t prev_upd = 0;
        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> diff;

        Embedding<uint32_t> embedding;

        start_phase1:

        if (tid == 0) {
            printf("Started executing phase 1...\n");
        }

        wait_b(&xsync_begin);

        while (curr_item < no_active) {
            get_work(tid, &thread_work[tid], no_active);

            if (thread_work[tid].start == thread_work[tid].stop) goto end_phase1;

            for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
                uint32_t src = edges[active[thread_work[tid].start]].src;
                uint32_t dst = edges[active[thread_work[tid].start]].dst;

                embedding.append(src);
                embedding.append(dst);

                explore_phase1(&embedding, 2, tid);

                embedding.pop();
                embedding.pop();

                prev_upd = per_thread_data[tid];
                if ((thread_work[tid].start <= 100000 && thread_work[tid].start % 10000 == 0) ||
                    (thread_work[tid].start <= 1000000 && thread_work[tid].start % 100000 == 0) ||
                    thread_work[tid].start % 1000000 == 0) {
                    printf("Processed %u / %lu\n", thread_work[tid].start, no_active);
                }
            }
        }

        end_phase1:

        end = std::chrono::high_resolution_clock::now();
        diff = end - start;
        printf("[PHASE1] Thread %d finished in %.3f seconds\n", tid, diff.count() / 1000.0);

        if (tid == 0) {
            // Reset start barrier for next step
            init_barrier(&xsync_begin, no_threads);
        }

        wait_b(&xsync_end);

        if (tid == 0) {
            printf("Done executing phase 1!\n");
            printf("Phase 1 runtime: %.3f seconds\n", diff.count() / 1000.0);
        }

        start_phase2:

        if (tid == 0) {
            // Reset end barrier
            init_barrier(&xsync_end, no_threads);

            no_triangles = 0;
            for (int i = 0; i < no_threads; i++) {
                no_triangles += per_thread_data[i];
                per_thread_data[i] = 0;
            }

            printf("Cache loaded with %lu elements!\n", no_triangles);

            curr_item = 0L;
            no_active = 0L;
            for (uint32_t i = 0; i < NB_NODES; i++) {
                if (algo->prefilter(i)) {
                    active[no_active++] = i;
                }
            }

            printf("Started executing phase 2...\n");
        }

        wait_b(&xsync_begin);

        start = std::chrono::high_resolution_clock::now();
        while (curr_item < no_active) {
            get_work(tid, &thread_work[tid], no_active);

            if (thread_work[tid].start == thread_work[tid].stop) goto end_phase2;

            for (; thread_work[tid].start < thread_work[tid].stop; thread_work[tid].start++) {
                explore_phase2(thread_work[tid].start, tid);

                prev_upd = per_thread_data[tid];

                if ((thread_work[tid].start <= 100000 && thread_work[tid].start % 10000 == 0) ||
                    (thread_work[tid].start <= 1000000 && thread_work[tid].start % 100000 == 0) ||
                    thread_work[tid].start % 1000000 == 0) {
                    printf("Processed %u / %lu\n", thread_work[tid].start, no_active);
                }
            }
        }

        end_phase2:

        end = std::chrono::high_resolution_clock::now();
        diff = end - start;
        printf("[PHASE2] Thread %d finished in %.3f seconds\n", tid, diff.count() / 1000.0);

        wait_b(&xsync_end);

        if (tid == 0) {
            no_triangles = 0;
            for (int i = 0; i < no_threads; i++) {
                no_triangles += per_thread_data[i];
                per_thread_data[i] = 0;
            }

            printf("Done executing phase 2!\n");
            printf("Phase 2 runtime: %.3f seconds\n", diff.count() / 1000.0);
        }
    }

    void execute_app() {
        for (int i = 0; i < no_threads - 1; i++) {
            threads[i] = new std::thread(&Engine<T>::compute, this, (void *) (i + 1));
        }

        compute(0);
    }

    void cancel_compute() {
        for (int i = 0; i < no_threads - 1; i++) {
            // pthread_cancel(threads[i]);
        }
    }


};