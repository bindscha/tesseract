#ifndef __COLOR_CLIQUE_HPP__
#define __COLOR_CLIQUE_HPP__

class ColorCliqueE{
public:
inline bool is_in_set(Embedding<uint32_t> embedding, uint32_t v_id, uint32_t step){

  if(embedding.contains(v_id)) return true;

  return false;
}

inline bool filter( const Embedding<VertexId>* embedding){ //const uint32_t cand, const Embedding<uint32_t>* embedding,const uint32_t step)const  {

  for(int i = 0; i < embedding->no_vertices() -1;i++){
      if( (*embedding)[i] % K == embedding->last() % K) return false;
      }

  return embedding->no_edges()  == ((embedding->no_vertices())* (embedding->no_vertices() -1 ))/2;

}

inline void process(Embedding<uint32_t>* embedding, uint32_t step) {

}
inline void process(const Embedding<uint32_t>* embedding, const uint32_t step, const int tid)  {

}
inline void process_update(Embedding<uint32_t>* embedding, uint32_t step) {

}
inline void process_update_tid(const Embedding<uint32_t>* embedding,const uint32_t step,const int tid) {

}
inline bool prefilter(const Embedding<uint32_t>* embedding,const uint32_t cand ) const {

  for(int i = 0; i < embedding->no_vertices();i++){
    if( (*embedding)[i] % K == cand % K) {
      return false;
    }
  }

  return degree[cand] >=K - 1;
}

inline bool prefilter(const uint32_t cand ) const {

  return degree[cand] >=K - 1;
}
inline bool expand(const uint32_t step) const {
  return step < K - 1;
}

    inline void output(){
      
    }
 void activate_nodes(){
  for (uint32_t i = 0; i < NB_NODES; i++) {
    if (prefilter(i))//should_be_active(i))
      for (size_t idx = 0; idx < degree[i]; idx++) {
        if (prefilter(edges_full[adj_offsets[i] + idx].dst) && edges_full[adj_offsets[i] + idx].dst >
                                                               i)
          active[no_active++] = adj_offsets[i] + idx;
      }
  }
}
};

#endif