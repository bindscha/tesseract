//
// Created by Jasmina Malicevic on 2019-08-12.
//

#ifndef TESSERACT_KSEARCH_H
#define TESSERACT_KSEARCH_H
//#define DEBUG_PRINT
#define NO_WORDS 3  
#define MOD_CHECK 0x1F //(NO_WORDS + 5)
#define MAX_HOPS 2
//Keyword search filter functions
class KSearchE {//}; public  Algorithm<Embedding<uint32_t>> {
    size_t no_cliques = 0;
    size_t total = 0;


    uint32_t** vertex_dist;
public:


    inline  bool pattern_filter(const Embedding<VertexId >* embedding,const VertexId cand ) const  {


        uint32_t col_found[NO_WORDS];
        uint32_t cand_col = cand & MOD_CHECK;
//        printf("Cand %u label is %u\n",cand, cand_col);
        if(embedding->no_vertices() == 0 )
            return cand_col < NO_WORDS;

        for(int i = 0; i < NO_WORDS;i++)
            col_found[i] =0;

        for (int i = 0; i < embedding->no_vertices(); i++) {
            uint32_t c_id = (*embedding)[i] & MOD_CHECK ;//< NO_WORDS;
            if (c_id < NO_WORDS ) {
                if (cand_col < NO_WORDS && cand_col == c_id) {
//                    printf("Not adding %u because it has the same label  (%u) as  %u (%u)\n",cand,cand_col,(*embedding)[i],c_id);
                    return false;
                }
                assert(c_id < NO_WORDS);
                col_found[c_id]++;

            }


        }

//        return true;
        if(cand_col < NO_WORDS) {
//            printf("Accepting*** %u\n",cand);
            return true;
        }
        int not_far = 0;
        for(int i = 0; i < NO_WORDS;i++){
//            assert(vertex_dist[cand][i] <=NB_EDGES);
            if(col_found[i] == 0) {
                if (vertex_dist[cand][i] <= K - embedding->no_vertices() && vertex_dist[cand][i] <= MAX_HOPS){
                    not_far++;
                    break;

//                    return false;
                }
//                else
//                    printf("Not adding %u because dist to %u is %u \n",cand,i,vertex_dist[cand][i]);
            }

        }
//        if(not_far){
//            printf("Accepting %u\n",cand);
//        }
        return not_far > 0;
//        return true;//(cols != 0 && not_far > 0) || cols == 0;


//        if(embedding->no_vertices() == 0 && cand_col >= NO_WORDS ) {
//#ifdef DEBUG_PRINT
//            printf("Skipping %u \n",cand);
//#endif
//            return false;
//        }
//        for(int i = 0; i < embedding->no_vertices();i++){
//            int col =  (*embedding)[i] % MOD_CHECK;
//            if(col < NO_WORDS && col == cand_col)
//
//                return false;
//            }

        return true;
    }

    inline  bool filter( const Embedding<uint32_t>* embedding) const  {
        if(embedding->no_vertices() < K){
            return true;
        }
        else if(embedding->no_vertices() == K){
            int cols = 0;
            for (int i = 0; i < embedding->no_vertices(); i++) {
                if (((*embedding)[i] & MOD_CHECK) < NO_WORDS)
                    cols++;
                if (cols == NO_WORDS) {
#ifdef DEBUG_PRINT
                    printf("Matched 0 :");
                    for (int k = 0; k < embedding->no_vertices(); k++) {
                        printf("%u(%u) ", (*embedding)[k], (*embedding)[k] & MOD_CHECK);
                    }
                    printf("\n");
#endif
                    return true;
                }
            }


#ifdef DEBUG_PRINT
            if(cols == NO_WORDS) {
                printf("Matched 1 :");
                for (int k = 0; k < embedding->no_vertices(); k++) {
                    printf("%u(%u) ", (*embedding)[k], (*embedding)[k] & MOD_CHECK);
                }
                printf("\n");
            }
#endif
                return cols == NO_WORDS;
            }


    }
    inline bool match(const Embedding<VertexId>* embedding) const{
        int cols = 0;
//        printf("\n ***Matching ... %u : ",embedding->no_vertices());
        for (int i = 0; i < embedding->no_vertices(); i++) {

            if (((*embedding)[i] & MOD_CHECK ) < NO_WORDS) {
//                printf(" %u ", (*embedding)[i]);
                cols++;
            }

            if (cols == NO_WORDS) {
#ifdef DEBUG_PRINT
                printf("Matched 2 (%u):",i);
                for (int k = 0; k < embedding->no_vertices(); k++) {
                    printf("%u(%u) ", (*embedding)[k], (*embedding)[k] & MOD_CHECK);
                }
                printf("\n");
#endif
                return true;
            }
        }
//        printf(" %d \n",cols);
        if (cols == NO_WORDS) {
#ifdef DEBUG_PRINT
            printf("Matched 3 :");
            for (int k = 0; k < embedding->no_vertices(); k++) {
                printf("%u(%u) ", (*embedding)[k], (*embedding)[k] & MOD_CHECK);
            }
            printf("\n");
#endif
            return true;
        }
        return cols == NO_WORDS;
    }
    inline bool getTS(const VertexId v_id, const Timestamp ts) const{
        return vertex_dist[v_id][NO_WORDS] == ts;
    }

    inline void pattern_update(const uint32_t v_id, const Timestamp ts_curr){
//        if(v_id % no_threads != tid) return;
//        if(vertex_dist[v_id][NO_WORDS] == ts_curr) return;
        int found_labs = 0;
        uint32_t own_lab = v_id & MOD_CHECK;
//        if( vertex_dist[v_id][own_lab] <  NB_EDGES  || vertex_dist[v_id][own_lab] ==  NB_EDGES+1) return;
        if(own_lab < NO_WORDS) { vertex_dist[v_id][own_lab] = 0; found_labs++;}
        int hops = 1;
        uint32_t dst,ts;
        FOREACH_EDGE_TS(v_id,dst,ts)

            uint32_t lab_dst = dst & MOD_CHECK;
            if(lab_dst != own_lab && lab_dst < NO_WORDS && hops < vertex_dist[v_id][lab_dst]){
                vertex_dist[v_id][lab_dst] = hops;
                found_labs++;
            }
            if(found_labs == NO_WORDS)break;
        ENDFOR

        while(found_labs < NO_WORDS && hops < MAX_HOPS ){
            hops++;
            uint32_t dst;
            FOREACH_EDGE_TS(v_id, dst,ts)
                uint32_t n,ts2;
            uint32_t own_lab = dst & MOD_CHECK;
                FOREACH_EDGE_TS(dst, n,ts2)
                    if(n == v_id) continue;
                    uint32_t lab_dst = n & MOD_CHECK;
                    if(lab_dst != own_lab && lab_dst < NO_WORDS && hops < vertex_dist[v_id][lab_dst]){

                        vertex_dist[v_id][lab_dst] = hops;
                        found_labs++;
                    }
                    if(found_labs == NO_WORDS)break;

                    ENDFOR
            ENDFOR
        }

      if(found_labs == NO_WORDS)
          vertex_dist[v_id][NO_WORDS] = ts_curr;
    }
    void printMap(){
        for(size_t i = 0; i < NB_NODES;i++){
            printf("%u: {",i);
            for(int j = 0; j < NO_WORDS;j++){
                printf("%u  ", vertex_dist[i][j]);
            }
            printf("}\n");
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

    inline void init(){
        vertex_dist = (uint32_t**) calloc(NB_NODES , sizeof(uint32_t*));
        for(size_t i = 0; i < NB_NODES;i++){
            vertex_dist[i] = (uint32_t*) calloc(sizeof(uint32_t),NO_WORDS + 1);
        }
        for(uint32_t i = 0; i < NB_NODES;i++){
            for(int j = 0; j < NO_WORDS; j++){
                vertex_dist[i][j] = NB_EDGES;
            }
            vertex_dist[i][NO_WORDS] = 0;
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