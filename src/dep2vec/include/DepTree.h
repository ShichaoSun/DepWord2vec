//
// Created by bruce on 7/13/16.
//

#ifndef DEPWORD2VEC_DEPTREE_H
#define DEPWORD2VEC_DEPTREE_H

#include "Vocab.h"
#include <vector>
#include <queue>
using namespace std;

#define MAX_SENTENCE_LENGTH 1024

struct TreeNode{
    int wordInVocab;
    int parent;
    vector<int> child;
};

class DepTree{
public:
    DepTree(const Vocab &v);
    void GetDepTreeFromFilePointer(FILE *fin);
    int GetWordInPos(int pos);
    int GetSenlen();
    int GetWordCount();
    vector<int> GetSample(int pos,int window);
private:
    int wordCount;
    int senlen;
    TreeNode deptree[MAX_SENTENCE_LENGTH+1];
    const Vocab &vocab;

    void ClearDepTree();
};


#endif //DEPWORD2VEC_DEPTREE_H
