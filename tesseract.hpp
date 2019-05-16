#ifndef __TESSERACT_HPP__
#define __TESSERACT_HPP__

#include "engine_emb.hpp"
void load_graph_and_init(char* i_fname, char* o_fname,bool do_up,size_t n_nodes,int n_threads,int algo_id=0,size_t bs=100000,size_t initial_chunk=0,int _w_id = 0,int _no_workers=1,bool _mmap=0 );
void load_graph_and_init(char* i_fname, char* o_fname,bool do_up,size_t n_nodes,int n_threads,int algo_id,size_t bs,size_t initial_chunk,int _w_id, int _no_workers,bool _mmap,char* _update_file);
void start_exec();

void print_progress();
#endif