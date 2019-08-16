#ifndef TRIANGLE_C_HPP
#define TRIANGLE_C_HPP

//#define K 3
//#define SIZE 5083604130LU //triangle uk 2005 directed
//#define SIZE 385730264LU //300LU triangles

//bool triangle_filter(uint32_t src, clique_vector set, uint32_t step){
//  if(step < K - 1) return true;
//
//  for(uint32_t i = 0; i <degree[src];i++){
//    uint32_t dst = edges[adj_offsets[src] + i].dst;
////should have a wrapper to loop over edges in one direction and hide the second condition from the end user
//    if(dst == set.buffer[0]) return true;
//
//  }
//  return false;
//}


//Clique finding filter functions
class TriangleC {//}; public  Algorithm<Embedding<uint32_t>> {
    size_t no_cliques = 0;
    size_t total = 0;
public:

    inline  bool pattern_filter(const Embedding<VertexId >* embedding,const VertexId cand ) const  {

        return  degree[cand] >= 1;
    }

    inline  bool filter( const Embedding<uint32_t>* embedding) const  {
        if(embedding->no_vertices() < K) return true;
        uint32_t dst,ts;

        FOREACH_EDGE_TS(embedding->last(),dst,ts)
//                if(ts <=set->ts_max)  
         if( ts <= embedding->max_ts())
                 if(dst ==  embedding->first() ) return true;
        ENDFOR
        return false;


    }
    inline bool match(const Embedding<VertexId>* embedding) const{
        return embedding->no_vertices() == K;
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
        if(!do_updates)
            for (uint32_t i = 0; i < NB_NODES; i++) {
                if (degree[i] >= 1)//should_be_active(i))
                    for (size_t idx = 0; idx < degree[i]; idx++) {
                        if (degree[edges_full[adj_offsets[i] + idx].dst] >= 1 && edges_full[adj_offsets[i] + idx].dst >
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
/*
class TriangleCount: public  Algorithm<Pattern<uint32_t, edge_full>> {

      bool is_in_set(Pattern<uint32_t, edge_full> set, uint32_t v_id, uint32_t step){

        for(uint32_t i = 0; i < step; i++){
          if(v_id == set.vertices_in_pattern[i] ) return true;
        }
        return false;
      }

      bool filter(uint32_t src, Pattern<uint32_t, edge_full>* set, uint32_t step) {
        if(step < K - 1) return true;
        uint32_t dst,ts;

//        FOREACH_EDGE_TS(src,dst,ts)
//                if(ts <=set->ts_max)
        FOREACH_EDGE_BACK(src,dst)
                if(dst == set->vertices_in_pattern[0] ) return true;
        ENDFOR
        return false;


      }

inline void process(Pattern <uint32_t, edge_full> *set, uint32_t step) {

}
inline void process(Pattern <uint32_t, edge_full> *set, uint32_t step,int tid) {

}
inline void process_update(Pattern <uint32_t, edge_full> *set, uint32_t step) {

}
inline void process_update_tid(Pattern <uint32_t, edge_full> *set, uint32_t step,int tid) {

}
      inline bool prefilter(uint32_t t){
        return true;
      }
      bool expand( uint32_t step) {
        return step < K - 1;
      }
};
#endif
 */