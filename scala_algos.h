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
    EmbeddingTmp* e ;
    inline void cpyEmb(const Embedding<VertexId>* embedding, EmbeddingTmp* e){

        e->edges_mat = embedding->getEdges();
        e->num_vertices = embedding->no_vertices();
        for(int i = 0; i < embedding->no_vertices(); i++){
            e->vertices[i] =((*embedding)[0]);
        }
        e->ts_mat = embedding->max_ts();
        e->ts = embedding->max_ts();

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

    inline  bool pattern_filter(const Embedding<VertexId >* embedding,const VertexId cand )  {
        EmbeddingTmp e;
        cpyEmb(embedding, &e);
        return algo.pfilter(&e, cand);

    }

    inline  bool filter( const Embedding<uint32_t>* embedding)  {
        EmbeddingTmp e;
        cpyEmb(embedding, &e);
        return algo.filter(&e);


    }
    void update(){
        algo.pupdate();
    }
    inline bool match(Embedding<VertexId>* embedding){
        EmbeddingTmp e;
        cpyEmb(embedding, &e);
        return algo.match(&e);
    }
    void init(){

    }
    inline void setItemsFound(size_t items){
        no_items = items;
    }


    inline void output(Embedding<VertexId>* embPre, Embedding<VertexId>* embPost){
        EmbeddingTmp e;
        cpyEmb(embPre, &e);
        EmbeddingTmp e2;
        cpyEmb(embPost, &e2);
        algo.output(&e,&e2);

        if(output_callback != NULL){
            output_callback(&e, 1);
        }
    }
    inline void output(Embedding<VertexId>* embedding){
        EmbeddingTmp e;
        cpyEmb(embedding, &e);
        algo.output_single(&e);

        if(output_callback != NULL){
            output_callback(embedding, 1);
        }
    }
    inline void output(Embedding<VertexId>* embedding, int tid){
        output(embedding);
    }

    void output_final(){
        printf("Found %lu items\n", no_items);
//        algo.output();
//        algo.output();
//TODO
    }
};

#endif //TESSERACT_SCALA_ALGOS_H

