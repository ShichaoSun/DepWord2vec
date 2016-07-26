//
// Created by bruce on 7/13/16.
//

#ifndef DEPWORD2VEC_DEPSKGNEG_H
#define DEPWORD2VEC_DEPSKGNEG_H

#include <pthread.h>

#include <math.h>
#include "Vocab.h"
#include "DepTree.h"

typedef float real;
#define EXP_TABLE_SIZE 1000
#define MAX_EXP 6

struct Para{
    void *pSelf;
    long long id;
};

class DepSkgNeg{
public:
    DepSkgNeg(const Vocab& v);
    void TrainModel();
    void SaveWordVectors(const char *output_file);

    void SetBinary(int x);
    void SetWindow(int x);
    void SetAlpha(real x);
    void SetSample(real x);
    void SetNegative(int x);
    void SetDebugmode(int x);
    void SetNumthread(int x);
    void Setlayer1_size(long long x);
    void SetTrainfile(const char *f);
    void SetIter(int x);

    ~DepSkgNeg();
private:
    const Vocab& vocab;
    real sample;
    real alpha;
    real *syn0;
    real *syn1neg;
    real *expTable;
    int *table;
    int debug_mode;
    int num_threads;
    int negative;
    int window;
    int binary;
    int iter;
    clock_t start;
    long long file_size;
    long long layer1_size;
    long long word_count_actual;
    char train_file[MAX_STRING];
    //Vocab v;
    const int table_size;
    real starting_alpha;
    void InitUnigramTable();
    void FindTreeStart(FILE *f);
    static void *BasicTrainModelThread(void *param);
    void TrainModelThread(long long id);
};

#endif //DEPWORD2VEC_DEPSKGNEG_H
