#ifndef __CLIQUES_OOC_HPP__
#define __CLIQUES_OOC_HPP__


//Clique finding filter functions
class CliqueFindE {//}; public  Algorithm<Embedding<uint32_t>> {
    size_t no_cliques = 0;
    size_t total = 0;
public:

    inline  bool prefilter(const Embedding<VertexId >* embedding,const VertexId cand )  {
        return degree[cand] >=K - 1;
    }

    inline  bool filter( const Embedding<uint32_t>* embedding)  {
        return embedding->no_edges()  == ((embedding->no_vertices())* (embedding->no_vertices() -1 ))/2;

    }
inline bool is_in_set(Embedding<uint32_t> embedding, uint32_t v_id, uint32_t step){

  if(embedding.contains(v_id)) return true;

  return false;
}


inline void setItemsFound(size_t items){
        no_cliques = items;
        total += no_cliques;
    }
inline void process(Embedding<uint32_t>* embedding, uint32_t step) {

}
inline void process(const Embedding<uint32_t>* embedding, const uint32_t step, const int tid)  {

}
inline void process_update(Embedding<uint32_t>* embedding, uint32_t step) {

}
inline void process_update_tid(const Embedding<uint32_t>* embedding,const uint32_t step,const int tid) {

}

inline bool expand(const uint32_t step)  {
  return step < K - 1;
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
    void output(){
      printf("[STAT] Found %lu %d-cliques (%lu total) \n", no_cliques, K, total);

    }
};

#endif