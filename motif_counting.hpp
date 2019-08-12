#ifndef __MOTIF_C_HPP__
#define __MOTIF_C_HPP__

#include<set>

int per_thread_patterns[256][256];
class MotifCountingE {
public:
    uint32_t mask = 1 << 31;

//__sync_fetch_and_add(&unique_patterns[tmp],1);
    inline bool filter(const Embedding<uint32_t>*embedding)const  {
        return true;
    }
    inline bool match(const Embedding<VertexId>* embedding) const{
        return embedding->no_vertices() == K;
    }
    inline bool expand(const uint32_t step) const {
        return step < K - 1;
    }
    inline void process_update(Embedding<uint32_t> *pattern, uint32_t step) {

    }
    inline void output(const Embedding<VertexId>* embedding){

    }
    inline void output(const Embedding<VertexId> *embedding, const int tid) {
//        uint32_t deg[K];

//        uint32_t deg2[K];
        std::array<uint32_t,K> deg;
        std::array<uint32_t,K> deg2;


        int no_edg = 0;
        for(int i = 0; i < embedding->no_vertices(); i++){
            deg[i] = embedding->vertex_degree_at_index(i);
#ifdef EDGE_TIMESTAMPS
            deg2[i] = embedding->old_vertex_degree_at_index(i);
            no_edg += embedding->old_vertex_degree_at_index(i);
#endif
        }
//        std::sort(deg, deg + K);


        std::sort(deg.begin(), deg.end());
        int pattern_id1 = 0;
        int i = 0;

        for(const auto &item: deg){

            pattern_id1 = pattern_id1 | (int)item;

            if(i == embedding->no_vertices() - 1 )break;
            i++;
            pattern_id1 = pattern_id1 << 2;
        }

        if((updateType == GraphUpdateType::EdgeDel) && do_updates) {
            per_thread_patterns[tid][pattern_id1]--;
        }
        else{
            per_thread_patterns[tid][pattern_id1]++;
        }
        if(!do_updates || GraphUpdateType::EdgeDel == updateType) return;
        int pattern_id2= 0;
        i = 0;
        if(no_edg/2 < (embedding->no_vertices()-1)) pattern_id2 = 0;
        else {
//            std::sort(deg2, deg2 + K);
            std::sort(deg2.begin(), deg2.end());

            for (const auto &item: deg2) {

                if (item == 0) {
                    pattern_id2 = 0;
                    break;
                }
                pattern_id2 = pattern_id2 | (int) item;

                if (i == embedding->no_vertices() - 1)break;
                i++;
                pattern_id2 = pattern_id2 << 2;
            }
        }

        if (pattern_id2 != 0 && pattern_id2 != pattern_id1) {
            per_thread_patterns[tid][pattern_id2]--;
        }

    }
    inline void process(const Embedding<uint32_t> *emb, const uint32_t step,const int tid)   {
//
//      std::array<uint32_t,K> deg;
//      for(int i = 0; i < emb->no_vertices(); i++){
//      deg[i] = emb->vertex_degree_at_index(i);
//      }
//      std::sort(deg.begin(),deg.end());
//      int pattern_id1 = 0;
//      int i = 0;
//      for(const auto &item: deg){
//      pattern_id1 = pattern_id1 | (int)item;
//      if(i == emb->no_vertices() - 1 )break;
//      i++;
//      pattern_id1 = pattern_id1 << 2;
//      }
//      per_thread_patterns[tid][pattern_id1]++;

    }

    inline bool pattern_filter(const Embedding<uint32_t>* embedding,const uint32_t cand ) const {
        return true;//embedding->no_vertices() <= K;// true;
    }
    void init() {
        no_active = 0;
        if(!do_updates)
            for(uint32_t i = 0; i < NB_NODES; i++){
                for(size_t idx = 0; idx < degree[i]; idx++) {
                    if(edges[adj_offsets[i] + idx].dst > i) {
                        active[no_active] = adj_offsets[i] + idx;
                        no_active++;
                    }
                }
            }
    }
    inline void setItemsFound(size_t items)const{

    }

    inline void output_final()const{
        int unique = 0;
        printf("[STAT] Patterns: ");
        for(int i = 0; i <256; i++){
            for(int j = 0; j <56;j++) {

                unique_patterns[i]+= per_thread_patterns[j][i];

            }

            if (unique_patterns[i] != 0) {
                unique++;
                printf("[%d]=%lu  ", i, unique_patterns[i]);
                unique_patterns[i] =0;
            }
        }
        printf("\n[STAT] Found %d unique patterns\n", unique);
    }
};
#endif
