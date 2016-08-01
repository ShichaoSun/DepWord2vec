//
// Created by bruce on 7/13/16.
//

#ifndef DEPWORD2VEC_VOCAB_H
#define DEPWORD2VEC_VOCAB_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define MAX_STRING 1024    //the max length of word

struct vocab_word{
    char *word;
    long long cn;
};

class Vocab {
public:
    Vocab();
    void LearnVocabFromTrainFile(const char *train_file);
    void SaveVocab(const char *save_vocab_file);
    void ReadVocab(const char *read_vocab_file);
    void SetMincount(int x);

    long long GetVocabSize() const;
    long long GetVocabWordCn(long long i) const;
    long long GetTotalWords() const;
    long long GetTrainTrees() const;
    char * GetVocabWord(long long a) const ;
    int SearchVocab(const char *word) const;// Returns position of a word in the vocabulary; if the word is not found, returns -1


    ~Vocab();
private:
    long long total_words;
    long long train_trees;
    long long vocab_size;
    long long vocab_max_size;
    const int vocab_hash_size;// Maximum 30 * 0.7 = 21M words in the vocabulary
    struct vocab_word *vocab;
    int min_reduce;
    int min_count;
    int *vocab_hash;
    // Reads a single word from a vocab file, assuming space + tab + EOL to be word boundaries
    void ReadWordFromVocab(char *word, FILE *fin);
    int ReadWordFromTrainFile(char *word1,char *word2,FILE *fin);
    int GetWordHash(const char *word) const;// Returns hash value of a word
    //int VocabCompare(const void*a,const void *b);
    int AddWordToVocab(char *word);// Adds a word to the vocabulary
    void SortVocab();
    void ReduceVocab();
    void ClearVocab();
};
#endif //DEPWORD2VEC_VOCAB_H
