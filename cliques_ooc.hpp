#ifndef __CLIQUES_OOC_HPP__
#define __CLIQUES_OOC_HPP__


//Clique finding filter functions
class CliqueFindE {//}; public  Algorithm<Embedding<uint32_t>> {
    size_t no_cliques = 0;
    size_t total = 0;
public:

    inline  bool pattern_filter(const Embedding<VertexId >* embedding,const VertexId cand )  {

        return  degree[cand] >=K - 1;
    }

    inline  bool filter( const Embedding<uint32_t>* embedding)  {
        return embedding->no_edges()  == ((embedding->no_vertices())* (embedding->no_vertices() -1 ))/2;

    }
    inline bool match(const Embedding<VertexId>* embedding) const{
        return embedding->no_vertices() == K;
    }
    inline bool is_in_set(Embedding<uint32_t> embedding, uint32_t v_id, uint32_t step){

        if(embedding.contains(v_id)) return true;

        return false;
    }


    inline void setItemsFound(size_t items){
        no_cliques = items;
//        if(do_updates && updateType == EdgeDel) total -= no_cliques;
//        else
            total+= no_cliques;
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

    inline void output(const Embedding<VertexId>* embedding, const int tid){
        output(embedding);
    }
    void init(){
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