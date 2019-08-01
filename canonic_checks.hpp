#ifndef __CANONIC_CHEKS_HPP__
#define __CANONIC_CHEKS_HPP__



inline bool canonic_check_r2_middle(const uint32_t v, const Embedding<VertexId>* embedding, const uint32_t step){
    uint32_t i = 0;
    bool foundNeighbour = false;
    uint32_t n_ts = embedding->max_ts();
    for(size_t i = 0; i < embedding->no_vertices(); ++i) {
        uint32_t u = (*embedding)[i];
        if(!foundNeighbour && has_edge_ts(u, v, embedding->max_ts(), &n_ts)) foundNeighbour = true;
        else
        if(foundNeighbour && u > v &&(( n_ts < embedding->max_ts() && u != (*embedding)[1] ) || n_ts == embedding->max_ts())) return false;
    }
    return true;
}

inline bool canonic_check_r1E(const uint32_t u,const Embedding<uint32_t>* embedding, const uint32_t step){
  return  (*embedding)[0] < u;
}

inline bool canonic_check_r2E_sym(const uint32_t v,const Embedding<uint32_t>* embedding, const uint32_t step)  {
  uint32_t i = 0;
  bool foundNeighbour = false;
    for(; i < embedding->no_vertices(); ++i) {
      const uint32_t u = (*embedding)[i];
    if (!foundNeighbour && has_edge_sym(u,v)) foundNeighbour = true;
    else
    if(foundNeighbour && u > v) return false;
  }
  return true;

}
inline bool canonic_check_r2E_nonsym(const uint32_t v,const Embedding<uint32_t>* embedding, const uint32_t step)  {
    uint32_t i = 0;
    bool foundNeighbour = false;
    for(; i < embedding->no_vertices(); ++i) {
        const uint32_t u = (*embedding)[i];
        if (!foundNeighbour && has_edge(u,v)) foundNeighbour = true;
        else
        if(foundNeighbour && u > v) return false;
    }
    return true;

}


#endif