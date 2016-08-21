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

struct vocab_cell{
    char *cell;
    long long cn;
};

class Vocab {
public:
    Vocab();

    void LearnVocabFromTrainFile(const char *train_file);

    void SaveVocab(const char *save_vocab_file);
    void ReadVocab(const char *read_vocab_file);

    void SetMincount(int x);

    unsigned int GetVocabWordSize() const;//Get vocab size
    unsigned int GetVocabWordPosSize() const;
    unsigned int GetVocabWordPosRelSize() const;
    unsigned int GetVocabRelWordPosSize() const;

    long long GetVocabWordCn(unsigned int i) const;// Get vocab cell count
    long long GetVocabWordPosCn(unsigned int i) const;
    long long GetVocabWordPosRelCn(unsigned int i) const;
    long long GetVocabRelWordPosCn(unsigned int i) const;

    unsigned int GetTotalWords() const;
    unsigned int GetTrainWords() const;
    unsigned int GetTrainTrees() const;

    char *GetVocabWord(unsigned int a) const;//Get vocab cell value
    char *GetVocabWordPos(unsigned int a) const;
    char *GetVocabWordPosRel(unsigned int a) const;
    char *GetVocabRelWordPos(unsigned int a) const;

    int SearchVocabWord(const char *word) const;// Returns position of a word in the vocabulary; if the word is not found, returns -1
    int SearchVocabWordPos(const char *wordPos) const;
    int SearchVocabWordPosRel(const char *wordPosRel) const;
    int SearchVocabRelWordPos(const char *relWordPos) const;

    ~Vocab();
private:
    unsigned int total_words;
    unsigned int train_words;
    unsigned int train_trees;

    unsigned int vocabWord_size;
    unsigned int vocabWordPos_size;
    unsigned int vocabWordPosRel_size;
    unsigned int vocabRelWordPos_size;

    unsigned int vocabWord_max_size;
    unsigned int vocabWordPos_max_size;
    unsigned int vocabWordPosRel_max_size;
    unsigned int vocabRelWordPos_max_size;

    const unsigned int vocab_hash_size;// Maximum 30 * 0.7 = 21M words in the vocabulary

    vocab_cell *vocabWord;
    vocab_cell *vocabWordPos;
    vocab_cell *vocabWordPosRel;
    vocab_cell *vocabRelWordPos;

    int min_reduce;
    int min_count;

    int *vocabWord_hash;
    int *vocabWordPos_hash;
    int *vocabWordPosRel_hash;
    int *vocabRelWordPos_hash;

    int GetWordHash(const char *word) const;// Returns hash value of a word
    int GetWordPosHash(const char *wordPos) const;
    int GetWordPosRelHash(const char *wordPosRel) const;
    int GetRelWordPosHash(const char *relWordPosRel) const;

    int AddWordToVocab(char *word);// Adds a word to the vocabulary
    int AddWordPosToVocab(char *wordPos);
    int AddWordPosRelToVocab(char *wordPosRel);
    int AddRelWordPosToVocab(char *relWordPos);

    void SortVocabWord();
    void SortVocabWordPos();
    void SortVocabWordPosRel();
    void SortVocabRelWordPos();

    void ReduceVocabWord();
    void ReduceVocabWordPos();
    void ReduceVocabWordPosRel();
    void ReduceVocabRelWordPos();
};
#endif //DEPWORD2VEC_VOCAB_H
