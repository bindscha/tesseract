#ifndef __ALGORITHM_HPP__
#define __ALGORITHM_HPP__

template<typename T>
class Algorithm{
public:

    virtual void activate_nodes(){};
    virtual bool filter( const uint32_t src,  const T*  set, const uint32_t step) const{};

    virtual void process_update(T*  set, uint32_t step){};

    virtual void process_update_tid(const T*  set, const uint32_t step, const int tid){} ;
//    virtual void process(T*  set, uint32_t step);
    virtual void process(const Embedding<uint32_t>* embedding, const uint32_t step, const int tid) {} ;

    virtual bool expand(const uint32_t step) const{};

    virtual bool prefilter(const uint32_t cand) const{};
    virtual inline bool prefilter(const Embedding<uint32_t>* embedding,const uint32_t cand ) const {
    }

    virtual void output()const{}
};



#endif