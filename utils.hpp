
#ifndef UTILS_H
#define UTILS_H
#include "common_include.hpp"



//#include "graph.hpp"




/*
 * Misc
 */
#ifdef __x86_64__
#define rdtscll(val) {                                           \
       unsigned int __a,__d;                                        \
       asm volatile("rdtsc" : "=a" (__a), "=d" (__d));              \
       (val) = ((unsigned long)__a) | (((unsigned long)__d)<<32);   \
}
#else
#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))
#endif
uint64_t get_cpu_freq(void);
void printTime(uint64_t start);
void printTime(uint64_t start, const char* msg);

//void init();
#endif
