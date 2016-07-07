//
// Created by bruce on 16-7-4.
//

#include "Word.h"


Word::Word(){
    word=NULL;
    cn=0;
}

Word::Word(const char *w){
    unsigned long length = strlen(w)+1;
    word=(char *)calloc(length,sizeof(char));
    strcpy(word,w);
    cn=0;
}

Word::Word(FILE *fin) {//just for read vocab from a file which is a saved vocab
    if (feof(fin)){
        word=NULL;
        cn=0;
    }
    word=(char *)calloc(MAX_STRING,sizeof(char));
    int a = 0,ch;
    while (!feof(fin)) {
        ch = fgetc(fin);
        if (ch == 13) continue;
        if ((ch == ' ') || (ch == '\t') || (ch == '\n')) {
            if (a > 0) {
                if (ch == '\n') ungetc(ch, fin);
                break;
            }
            if (ch == '\n') {
                strcpy(word, (char *)"</s>");
                return;
            } else continue;
        }
        word[a] = (char)ch;
        a++;
        if (a >= MAX_STRING - 1) a--;   // Truncate too long words
    }
    word[a] = 0;
    char c;
    fscanf(fin, "%lld%c", &cn, &c);
}

void Word::CNincrement(){
    cn++;
}

const long long Word::GetCN(){
    return cn;
}

char* Word::GetWord(){
    return word;
}

void Word::CreatWordSpace(int length){
    if(length>=0)
        word=(char *)calloc((unsigned)length, sizeof(char));
}

void Word::SetWord(const char *w) {
    if(word==NULL)
        word=(char*)(char *)calloc(MAX_STRING,sizeof(char));
    strcpy(word, w);
}

void Word::SetCN(int CN) {
    cn=CN;
}

void Word::ReadWord(FILE *fin) {// Reads a single word from a file, assuming space + tab + EOL to be word boundaries
    if(word!=NULL) free(word);
    word=(char *)calloc(MAX_STRING,sizeof(char));
    int a = 0, ch;
    while (!feof(fin)) {
        ch = fgetc(fin);
        if (ch == 13) continue;
        if ((ch == ' ') || (ch == '\t') || (ch == '\n')) {
            if (a > 0) {
                if (ch == '\n') ungetc(ch, fin);
                break;
            }
            if (ch == '\n') {
                strcpy(word, (char *)"</s>");
                return;
            } else continue;
        }
        word[a] = (char)ch;
        a++;
        if (a >= MAX_STRING - 1) a--;   // Truncate too long words
    }
    word[a] = 0;
}

void Word::DeleteWord(){
    free(word);
    word=NULL;
}