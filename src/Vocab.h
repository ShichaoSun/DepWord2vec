//
// Created by bruce on 16-7-4.
//

#ifndef DEPWORD2VEC_VOCAB_H
#define DEPWORD2VEC_VOCAB_H


#include "Word.h"


class Vocab {
public:
    Vocab();
    long long GetVocabSize();
    long long GetVocabWordCn(long long i);
    long long GetTrainWords();
    Word* GetVocab();
    int SearchVocab(const char *word);// Returns position of a word in the vocabulary; if the word is not found, returns -1
    int AddWordToVocab(char *word);// Adds a word to the vocabulary
    int AddWordToVocab(Word *w);
    void LearnVocabFromTrainFile(const char *train_file);
    void SaveVocab(const char *save_vocab_file);
    void ReadVocab(const char *read_vocab_file);
    void DeleteVocab();
private:
    long long train_words;
    long long vocab_size;
    long long vocab_max_size;
    const int vocab_hash_size;// Maximum 30 * 0.7 = 21M words in the vocabulary
    Word* vocab;
    int min_reduce;
    int min_count;
    int *vocab_hash;
    int GetWordHash(const char *word);// Returns hash value of a word
    //int VocabCompare(const void*a,const void *b);
    void SortVocab();
    void ReduceVocab();
    void DestroyVocab();
};


#endif //DEPWORD2VEC_VOCAB_H
