//
// Created by bruce on 7/4/16.
//

#ifndef DEPWORD2VEC_SKGNEG_H
#define DEPWORD2VEC_SKGNEG_H

#include <pthread.h>

#include <math.h>
#include "Vocab.h"

typedef float real;
#define EXP_TABLE_SIZE 1000
#define MAX_EXP 6
#define MAX_SENTENCE_LENGTH 1000

struct Para{
    void *pSelf;
    long long id;
};


class SkgNeg {
public:
    SkgNeg(const Vocab& v);
    void TrainModel();
    void SaveWordVectors(char *output_file);

    void SetBinary(int x);
    void SetWindow(int x);
    void SetAlpha(real x);
    void SetSample(real x);
    void SetNegative(int x);
    void SetDebugmode(int x);
    void SetNumthread(int x);
    void Setlayer1_size(long long x);
    void SetTrainfile(const char *f);

    ~SkgNeg();
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
    clock_t start;
    long long file_size;
    long long layer1_size;
    long long word_count_actual;
    char train_file[MAX_STRING];
    //Vocab v;
    const int table_size;
    real starting_alpha;
    void InitUnigramTable();
    static void *BasicTrainModelThread(void *param);
    void TrainModelThread(long long id);
    int ReadWordIndex(FILE *fin);
};


#endif //DEPWORD2VEC_SKGNEG_H
