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
    int senlen;
    TreeNode deptree[MAX_SENTENCE_LENGTH+1];

    DepTree();
    int GetDepTreeFromFilePointer(FILE *fin);
private:
    void ClearDepTree();// clear the tree
};

#endif //DEPWORD2VEC_DEPTREE_H
