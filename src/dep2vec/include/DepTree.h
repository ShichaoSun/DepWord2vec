//
// Created by bruce on 7/13/16.
//

#ifndef DEPWORD2VEC_DEPTREE_H
#define DEPWORD2VEC_DEPTREE_H

#include "Vocab.h"
#include <vector>
#include <queue>
#include <math.h>

using namespace std;

typedef float real;

#define MAX_SENTENCE_LENGTH 1024

struct TreeNode{
    char wordPos[MAX_STRING];
    char toRel[MAX_STRING];
    int parent;
    vector<int> child;
};

class DepTree{
public:
    DepTree(const Vocab &v);
    void GetDepTreeFromFilePointer(FILE *fin);
    int GetWordInPos(int pos);
    int GetSenlen();
    int GetWordCountActual();
    vector<int> GetSample(int pos,int window,real sample, unsigned long long &next_random);// get sample
private:
    int wordCountActual;
    int senlen;
    TreeNode deptree[MAX_SENTENCE_LENGTH+1];
    const Vocab &vocab;

    int SubSampling(int pos,int *visited,queue<int> &q,vector<int> &sam,real sample,unsigned long long &next_random);// recursive subsampling
    void ClearDepTree();// clear the tree
};


#endif //DEPWORD2VEC_DEPTREE_H
