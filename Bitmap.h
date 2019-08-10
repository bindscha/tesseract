//
// Created by Jasmina Malicevic on 2019-08-05.
//

#ifndef TESSERACT_BITMAP_H
#define TESSERACT_BITMAP_H

#define WORD_OFFSET(i) ((i) >> 6)
#define BIT_OFFSET(i) ((i) & 0x3f)

class Bitmap {
public:
    size_t size;
    unsigned long data[WORD_OFFSET(100000)];
    Bitmap() : size(0) { }
    Bitmap(size_t size) : size(size) {
//        data = new unsigned long [WORD_OFFSET(size)+1];
        clear();
    }
    Bitmap(const Bitmap* bitset, size_t size):size(size){
//        data = new unsigned long [WORD_OFFSET(size)+1];
        memcpy(data,bitset->data,(WORD_OFFSET(size)+1 )* sizeof(unsigned long));
    }
    ~Bitmap() {
//        delete [] data;
    }
   inline void clear() {
        size_t bm_size = WORD_OFFSET(size);
#pragma omp parallel for
        for (size_t i=0;i<=bm_size;i++) {
            data[i] = 0;
        }
    }
    void fill() {
        size_t bm_size = WORD_OFFSET(size);
#pragma omp parallel for
        for (size_t i=0;i<bm_size;i++) {
            data[i] = 0xffffffffffffffff;
        }
        data[bm_size] = 0;
        for (size_t i=(bm_size<<6);i<size;i++) {
            data[bm_size] |= 1ul << BIT_OFFSET(i);
        }
    }
    inline bool get_bit(size_t i)const {
        return data[WORD_OFFSET(i)] & (1ul<<BIT_OFFSET(i));
    }
    inline void set_bit(size_t i)  {

        data[WORD_OFFSET(i)] |= (1ul<<BIT_OFFSET(i));
//        __sync_fetch_and_or(data+WORD_OFFSET(i), 1ul<<BIT_OFFSET(i));
    }
};
#endif //TESSERACT_BITMAP_H
