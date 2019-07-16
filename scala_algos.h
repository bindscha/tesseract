//
// Created by jasmi on 7/4/2019.
//

#ifndef TESSERACT_SCALA_ALGOS_H
#define TESSERACT_SCALA_ALGOS_H

//#include "embedding.hpp"
//#include "libtesseract.h"
//Clique finding filter functions
class ScalaAlgo {//}; public  Algorithm<Embedding<uint32_t>> {
    Algorithm algo;
    static bool symmetric;
    size_t no_items = 0;
    inline EmbeddingTmp* cpyEmb(const Embedding<VertexId>* embedding){
        EmbeddingTmp* e ;
        e->edges_mat = embedding->getEdges();
        e->num_vertices = embedding->no_vertices();
        for(int i = 0; i < embedding->no_vertices(); i++){
            e->vertices[i] =((*embedding)[0]);
        }
        e->ts_mat = embedding->max_ts();
        e->ts = embedding->max_ts();
        return e;
    }

public:
    void setAlgo(Algorithm* _algorithm){
        algorithm.pupdate = _algorithm->pupdate;
        algorithm.pfilter = _algorithm->pfilter;
        algorithm.filter = _algorithm->filter;
        algorithm.match = _algorithm->match;
        algorithm.output = _algorithm->output;
        algorithm.init = _algorithm->init;
    }
   static inline void setSymmetric(bool sym){
        symmetric = sym;
    }
    static inline bool getSymmetric(){
        return ScalaAlgo::symmetric;
    }
    inline  bool pattern_filter(const Embedding<VertexId >* embedding,const VertexId cand )  {

        return algo.pfilter(cpyEmb(embedding), cand);

    }

    inline  bool filter( const Embedding<uint32_t>* embedding)  {
        return algo.filter(cpyEmb(embedding));


    }
    inline void process(Embedding<uint32_t>* embedding, uint32_t step) {

    }

    void update(){
        algo.pupdate();
    }
    inline bool match(Embedding<VertexId>* embedding){
        return algo.match(cpyEmb(embedding));
    }
    void init(){

    }
    inline void setItemsFound(size_t items){
        no_items = items;
    }

    void output(Embedding<VertexId>* embPre, Embedding<VertexId>* embPost){
        algo.output(cpyEmb(embPre), cpyEmb(embPost));
    }
    void output(Embedding<VertexId>* embPre){

    }
    void output_final(){
        printf("Found %lu items\n", no_items);
//        algo.output();
//        algo.output();
//TODO
    }
};
bool ScalaAlgo::symmetric = false;

#endif //TESSERACT_SCALA_ALGOS_H

