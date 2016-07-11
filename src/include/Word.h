//
// Created by bruce on 16-7-4.
//

#ifndef DEPWORD2VEC_WORD_H
#define DEPWORD2VEC_WORD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING 100    //the max length of word

class Word {
public:
    Word();
    Word(const char *w);
    Word(FILE *fin);
    // Reads a single word from a saved vocab file, assuming space + tab + EOL to be word boundaries
    void CNincrement();
    void CreatWordSpace(int length);
    void SetWord(const char *w);
    void SetCN(int CN);
    const long long GetCN();
    char* GetWord();
    void ReadWord(FILE *fin);
    void DeleteWord();
private:
    char *word;
    long long cn; //count number
};


#endif //DEPWORD2VEC_WORD_H
