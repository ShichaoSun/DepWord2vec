//
// Created by bruce on 7/13/16.
//
#include "DepTree.h"

DepTree::DepTree(const Vocab &v):vocab(v){
    senlen=0;
}

void DepTree::DeleteDepTree() {
    for(int i=0;i<senlen;i++){
        deptree[i].parent=0;
        deptree[i].wordInVocab=0;
        deptree[i].child.clear();
    }
}

void DepTree::GetDepTreeFromFilePointer(FILE *fin){
    DeleteDepTree();
    char temp[MAX_STRING];
    char *p;
    const char *d=" ";
    fgets(temp,MAX_STRING,fin);
    senlen=atoi(temp);
    int sen_pos=1;
    for(int i=0;i<senlen;i++){
        fgets(temp,MAX_STRING,fin);
        char ssss[MAX_STRING];
        strcpy(ssss,temp);
        p = strtok(temp, d);
        int parent,child;
        for(int j=0;j<5;j++){
            if(j==2)
                parent=atoi(p);
            if(j==3)
                deptree[sen_pos].wordInVocab = vocab.SearchVocab(p);
            if(j==4)
                child = atoi(p);
            p=strtok(NULL,d);
        }
        deptree[child].parent=parent;
        deptree[parent].child.push_back(child);
    }
}

int DepTree::GetWordInPos(int pos) {
    return deptree[pos].wordInVocab;
}

int DepTree::GetSenlen() {
    return senlen;
}

vector<int> DepTree::GetSample(int pos,int window){
    vector<int> sam;
    int p=deptree[pos].parent;
    //to only parent
    for(int j=0;j<window;j++){//down to top sample
        if(p==0)
            break;
        sam.push_back(p);
        p=deptree[p].parent;
    }
    queue<int> q;
    if(deptree[pos].child.empty()){
        return sam;
    }
    //to child
    for(int j=0;j<deptree[pos].child.size();j++){
        q.push(deptree[pos].child[j]);
    }
    for(int j=0;j<window;j++){
        int qsize=q.size();
        for(int k=0;k<qsize;k++){
            int t=q.front();
            sam.push_back(t);
            q.pop();
            if(deptree[t].child.empty())
                continue;
            for(int i=0;i<deptree[t].child.size();i++){
                q.push(deptree[t].child[i]);
            }
        }
    }
    return sam;
}
