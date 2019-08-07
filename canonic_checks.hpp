#ifndef __CANONIC_CHEKS_HPP__
#define __CANONIC_CHEKS_HPP__



inline bool canonic_check_r2_middle(const uint32_t v, const Embedding<VertexId> *embedding) {
//    uint32_t i = 0;
    bool foundNeighbour = false;
    Timestamp n_ts;
    bool old_ts = false;
    for (size_t i = 0; i < embedding->no_vertices() - 1; i++) {
        uint32_t u = (*embedding)[i];
//        if (!foundNeighbour) {
//            if (embedding->contains_edge_at_indices(embedding->no_vertices() - 1,i)) {
//                foundNeighbour = true;
//                if (!embedding->edge_at_indices_is_new(embedding->no_vertices() - 1, i))
//                    old_ts = true;
//            }
//        } else {
//            if (foundNeighbour && u > v && (!old_ts || (old_ts && u != (*embedding)[1])))// == embedding->max_ts()))
//                return false;
//        }
//    }
//
//    return true;
//}
////        bool ts_smaller = false;
        if(!foundNeighbour){

            uint32_t tmp_dst, ts_t;
            if(degree[u] < degree[v]) {
                FOREACH_EDGE_TS(u, tmp_dst, ts_t)
                    if (v == tmp_dst) {
                        if (ts_t <= embedding->max_ts()) {
                            n_ts = ts_t;
                            foundNeighbour = true;
                            break;
                        }
                    }
//    if(ts_t > ts) break;
                ENDFOR
            }
            else{
                FOREACH_EDGE_TS(v, tmp_dst, ts_t)
                    if (u == tmp_dst) {
                        if (ts_t <= embedding->max_ts()) {
                            n_ts = ts_t;
                            foundNeighbour = true;
                            break;
                        }
                    }
//    if(ts_t > ts) break;
                ENDFOR
            }
        }
//        if(!foundNeighbour && has_edge_ts(u, v, embedding->max_ts(), &n_ts)) foundNeighbour = true;
        else
        if(foundNeighbour && u > v &&(( n_ts < embedding->max_ts() && u != (*embedding)[1] ) || n_ts == embedding->max_ts())) return false;
    }
    return true;
}

inline bool canonic_check_r1E(const uint32_t u,const Embedding<uint32_t>* embedding, const uint32_t step) {
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