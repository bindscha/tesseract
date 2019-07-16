#ifndef __COLOR_CLIQUE_HPP__
#define __COLOR_CLIQUE_HPP__

class ColorCliqueE{
    size_t no_cliques = 0;

    size_t total = 0;
public:
    inline bool is_in_set(Embedding<uint32_t> embedding, uint32_t v_id, uint32_t step){

        if(embedding.contains(v_id)) return true;

        return false;
    }

    inline bool filter( const Embedding<VertexId>* embedding){ //const uint32_t cand, const Embedding<uint32_t>* embedding,const uint32_t step)const  {

//        for(int i = 0; i < embedding->no_vertices() -1;i++){
//            if( (*embedding)[i] % K == embedding->last() % K) return false;
//        }

        return embedding->no_edges()  == ((embedding->no_vertices())* (embedding->no_vertices() -1 ))/2;

    }

    inline void output(const Embedding<VertexId>* embedding, const int tid){

    }
    inline void output(const Embedding<VertexId>* embedding){
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

    inline bool match(const Embedding<VertexId>* embedding) const{
        return embedding->no_vertices() == K;
    }
    inline bool pattern_filter(const Embedding<uint32_t>* embedding,const uint32_t cand ) const {

        if(embedding->no_vertices() == K || degree[cand] < K -1 )  return false;
        for(int i = 0; i < embedding->no_vertices();i++){
            if( (*embedding)[i] % K == cand % K) {
                return false;
            }
        }
        return true;
    }

    inline bool pattern_filter(const uint32_t cand ) const {

        return degree[cand] >=K - 1;
    }
    inline void setItemsFound(size_t items){
        no_cliques = items;
        total += items;
    }
    void output_final(){
        printf("[STAT] Found %lu %d-cliques (%lu total) \n", no_cliques, K, total);

    }
    void init(){
        if(!do_updates)
            for (uint32_t i = 0; i < NB_NODES; i++) {
                if (pattern_filter(i))//should_be_active(i))
                    for (size_t idx = 0; idx < degree[i]; idx++) {
                        if (pattern_filter(edges_full[adj_offsets[i] + idx].dst) && edges_full[adj_offsets[i] + idx].dst >
                                                                                    i)
                            active[no_active++] = adj_offsets[i] + idx;
                    }
            }
    }
};

#endif