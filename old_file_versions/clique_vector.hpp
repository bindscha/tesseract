#ifndef __CLIQUE_VECTOR_H__
#define __CLIQUE_VECTOR_H__
#include <array>
#include <iostream>



//const int K = 3; // number of cliques
//#define SIZE 5083604130LU //triangle uk 2005 directed
#define SIZE 639916817LU //300LU triangles
//#define SIZE 9933532019LU // 4-clique
//#define SIZE 10218984470LU
//5cliques 467429836174
//class clique_vector{



class clique_vector{
public:
    std::array<uint32_t, K> buffer;

    void operator=(clique_vector cpy) {
      int idx = 0;
      for (int idx = 0; idx < cpy.buffer.size(); idx++) {
        buffer[idx] = cpy.buffer[idx];


      }
    }
    clique_vector(clique_vector cpy , uint32_t item){
        int idx = 0;
      for(int idx = 0 ; idx<cpy.buffer.size(); idx++){
        buffer[idx] = cpy.buffer[idx];
        }

        buffer[idx] = item;

    }

    clique_vector(uint32_t a1, uint32_t b1, uint32_t c1 ){
      buffer[0] = a1;//.push_back(a1);
      buffer[1] = b1;//.push_back(b1);
      buffer[2] = c1;//.push_back(b1);
      buffer[3] = -1U;
    }

};




#endif