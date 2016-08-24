//
// Created by bruce on 7/13/16.
//

#ifndef DEPWORD2VEC_DEPSKGNEG_H
#define DEPWORD2VEC_DEPSKGNEG_H

#include <pthread.h>

#include "DepTree.h"

#define EXP_TABLE_SIZE 1000
#define MAX_EXP 6

struct Para{
    void *pSelf;
    int id;
};

class DepSkgNeg{
public:
    DepSkgNeg(const Vocab& v);
    void TrainModel();
    void SaveWordVectors(const char *output_file);

    void SetBinary(int x);
    void SetBigdata(int x);
    void SetWindow(int x);
    void SetAlpha(real x);
    void SetSample(real x);
    void SetNegative(int x);
    void SetDebugmode(int x);
    void SetNumthread(int x);
    void Setlayer1_size(unsigned int x);
    void SetTrainfile(const char *f);
    void SetIter(int x);

private:
    const Vocab& vocab;
    real sample;
    real starting_alpha;
    real alpha;
    real *syn0Word;
    real *syn1Word;
    real *syn1WordPos;
    real *syn1WordPosRel;
    real *syn1RelWordPos;
    real *expTable;
    int *table;
    int debug_mode;
    int bigdata;
    int num_threads;
    int negative;
    int window;
    int binary;
    int iter;
    clock_t start;
    long long file_size;
    unsigned int layer1_size;
    unsigned int word_count_total;
    unsigned int tree_count_total;

    char train_file[MAX_STRING];
    //Vocab v;
    const int table_size;
    void InitUnigramTable();
    void FindTreeStart(FILE *f);
    static void *BasicTrainModelThread(void *param);
    void TrainModelThread(int id);
};

#endif //DEPWORD2VEC_DEPSKGNEG_H
