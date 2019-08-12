//
// Created by Jasmina Malicevic on 2019-08-12.
//

#ifndef TESSERACT_KSEARCH_H
#define TESSERACT_KSEARCH_H

#define NO_WORDS 3
#define MOD_CHECK (NO_WORDS + 5)
#define MAX_HOPS 2
//Keyword search filter functions
class KSearchE {//}; public  Algorithm<Embedding<uint32_t>> {
    size_t no_cliques = 0;
    size_t total = 0;


    int** vertex_dist;
public:

    inline  bool pattern_filter(const Embedding<VertexId >* embedding,const VertexId cand ) const  {

        int cand_col = cand % MOD_CHECK;
        if(embedding->no_vertices() == 0 && cand_col >= NO_WORDS ) {
#ifdef DEBUG_PRINT
            printf("Skipping %u \n",cand);
#endif
            return false;
        }
        for(int i = 0; i < embedding->no_vertices();i++){
            int col =  (*embedding)[i] % MOD_CHECK;
            if(col < NO_WORDS && col == cand_col)

                return false;
            }

        return true;
    }

    inline  bool filter( const Embedding<uint32_t>* embedding) const  {
        if(embedding->no_vertices() < K){
            return true;
        }
        else if(embedding->no_vertices() == K){
            int cols = 0;
            for (int i = 0; i < embedding->no_vertices(); i++) {
                if ((*embedding)[i] % MOD_CHECK < NO_WORDS)
                    cols++;
                if (cols == NO_WORDS) {
#ifdef DEBUG_PRINT
                    printf("Matched:");
                    for (int k = 0; k < embedding->no_vertices(); k++) {
                        printf("%u(%u) ", (*embedding)[k], (*embedding)[k] % MOD_CHECK);
                    }
                    printf("\n");
#endif
                    return true;
                }
            }
            {
#ifdef DEBUG_PRINT
                printf("Matched:");
                    for (int k = 0; k < embedding->no_vertices(); k++) {
                        printf("%u(%u) ", (*embedding)[k], (*embedding)[k] % MOD_CHECK);
                    }
                    printf("\n");
#endif
                return cols == NO_WORDS;
            }
        }

    }
    inline bool match(const Embedding<VertexId>* embedding) const{
        int cols = 0;
        for (int i = 0; i < embedding->no_vertices(); i++) {
            if ((*embedding)[i] % MOD_CHECK < NO_WORDS)
                cols++;
            if (cols == NO_WORDS) {
#ifdef DEBUG_PRINT
                printf("Matched1 (%u):",i);
                for (int k = 0; k < embedding->no_vertices(); k++) {
                    printf("%u(%u) ", (*embedding)[k], (*embedding)[k] % MOD_CHECK);
                }
                printf("\n");
#endif
                return true;
            }
        }
        if (cols == NO_WORDS) {
#ifdef DEBUG_PRINT
            printf("Matched:");
            for (int k = 0; k < embedding->no_vertices(); k++) {
                printf("%u(%u) ", (*embedding)[k], (*embedding)[k] % MOD_CHECK);
            }
            printf("\n");
#endif
            return true;
        }
        return cols == NO_WORDS;
    }

    inline void pattern_update(const uint32_t v_id, const int tid, const int no_threads){
//        if(v_id % no_threads != tid) return;


        int found_labs = 0;
        int own_lab = v_id % MOD_CHECK;
        if(own_lab < NO_WORDS) { found_labs++;}
        int hops = 1;
        uint32_t dst;
        FOREACH_EDGE(v_id,dst)

            int lab_dst = dst % MOD_CHECK;
            if(lab_dst != own_lab && lab_dst < NO_WORDS && lab_dst < vertex_dist[v_id][lab_dst]){
                vertex_dist[v_id][lab_dst] = hops;
                found_labs++;
            }
            if(found_labs == NO_WORDS)break;
        ENDFOR

        while(found_labs < NO_WORDS && hops < MAX_HOPS ){
            hops++;
            uint32_t dst;
            FOREACH_EDGE(v_id, dst)
                uint32_t n;
                FOREACH_EDGE(dst, n)
                    if(n == v_id) continue;
                    int lab_dst = dst % MOD_CHECK;
                    if(lab_dst != own_lab && lab_dst < NO_WORDS && lab_dst < vertex_dist[v_id][lab_dst]){

                        vertex_dist[v_id][lab_dst] = hops;
                        found_labs++;
                    }
                    if(found_labs == NO_WORDS)break;

                    ENDFOR
            ENDFOR
        }

    }


    inline void output(const Embedding<VertexId>* embedding) const{
        int no_edg = 0;
        for(int i = 0; i < embedding->no_vertices(); i++){
#ifdef EDGE_TIMESTAMPS
            no_edg += embedding->old_vertex_degree_at_index(i);
#endif
        }

        if(no_edg  == ((embedding->no_vertices())* (embedding->no_vertices() -1 ))/2) {
            //TODO print modified
        }
        else{
            //TODO print new if update type == ADD
            //TODO print del if update type == DEL
        }
        //TODO VertexDel is covered by this, but we  can add special case if need be
    }
    inline void output(const Embedding<VertexId>* embedding, const int tid) const{
        output(embedding);
    }
    inline void setItemsFound(size_t items){
        no_cliques = items;
//        if(do_updates && updateType == EdgeDel) total -= no_cliques;
//        else
        total+= no_cliques;
    }

    void init(){
        vertex_dist = (int**) calloc(NB_NODES , sizeof(int*));
        for(size_t i = 0; i < NB_NODES;i++){
            vertex_dist[i] = (int*) calloc(sizeof(int),NO_WORDS);
        }
        for(uint32_t i = 0; i < NB_NODES;i++){
            for(int j = 0; j < NO_WORDS; j++){
                vertex_dist[i][j] = NB_EDGES;
            }
        }
        if(!do_updates)
            for (uint32_t i = 0; i < NB_NODES; i++) {
                if (degree[i] >=K - 1)//should_be_active(i))
                    for (size_t idx = 0; idx < degree[i]; idx++) {
                        if (degree[edges_full[adj_offsets[i] + idx].dst] >=K - 1 && edges_full[adj_offsets[i] + idx].dst >
                                                                                    i)
                            active[no_active++] = adj_offsets[i] + idx;
                    }
            }
    }
    void output_final(){
        printf("[STAT] Found %lu %d-cliques (%lu total) \n", no_cliques, K, total);

    }
};

#endif