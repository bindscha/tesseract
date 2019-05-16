//#include<stdint.h>
//#include<stdio.h>
//#include<string.h>
#include"utils.hpp"
uint64_t get_cpu_freq(void) {
   FILE *fd;
   uint64_t freq = 0;
   float freqf = 0;
   char *line = NULL;
   size_t len = 0;

   fd = fopen("/proc/cpuinfo", "r");
   if (!fd) {
      fprintf(stderr, "failed to get cpu frequency\n");
      perror(NULL);
      return freq;
   }

   while (getline(&line, &len, fd) != EOF) {
      if (sscanf(line, "cpu MHz\t: %f", &freqf) == 1) {
         freqf = freqf * 1000000UL;
         freq = (uint64_t) freqf;
         break;
      }
   }

   fclose(fd);
   return freq;
}

void printTime(uint64_t start){
   uint64_t stop;
   rdtscll (stop);

   printf("%lu %.4f\n",(stop-start),
          (double)(stop - start)/(double)(2400LU*1000));
}
void printTime(uint64_t start, const char* msg){
   uint64_t stop;
   rdtscll (stop);
   printf("%s: %lu = %lums\n",msg,
          (stop - start),
          (stop - start)/(2400LU*1000));
}



