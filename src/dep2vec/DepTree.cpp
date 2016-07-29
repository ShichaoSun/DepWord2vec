//
// Created by bruce on 7/13/16.
//
#include "DepTree.h"

DepTree::DepTree(const Vocab &v):vocab(v){
    senlen=-1;
    wordCount=0;
}

void DepTree::ClearDepTree() {
    for(int i=0;i<=senlen;i++){
        deptree[i].parent=-1;
        deptree[i].wordInVocab=-2;
        deptree[i].child.clear();
    }
    senlen=-1;
    wordCount=0;
}

void DepTree::GetDepTreeFromFilePointer(FILE *fin){
    ClearDepTree();  //clear
    char temp[MAX_STRING];
    char *p;
    const char *d=" ";
    while (true){
        if(feof(fin))
            return;
        fgets(temp,MAX_STRING,fin);
        if(strlen(temp)<2)
            continue;
        int j=0;
        for(j=0;j<strlen(temp)-1;j++)
            if(!isdigit(temp[j]))
                break;
        if(j==strlen(temp)-1 && temp[j]=='\n')
            break;
    }
    senlen=atoi(temp);
    assert(senlen>0);
    wordCount=senlen;

    for(int i=0;i<senlen;i++){
        assert(!feof(fin));
        fgets(temp,MAX_STRING,fin);

        char *q=temp;

        p = strsep(&q, d);
        assert(strlen(p)>0);

        p = strsep(&q, d);
        assert(strlen(p)>0);
        int parentInVocab=vocab.SearchVocab(p);

        p = strsep(&q, d);
        assert(strlen(p)>0);
        int parentInSen=atoi(p);
        assert(parentInSen>-1);

        p = strsep(&q, d);
        assert(strlen(p)>0);
        int childInVocab=vocab.SearchVocab(p);
        if(childInVocab<0)
            wordCount--;

        p = strsep(&q, d);
        assert(strlen(p)>0);
        int childInSen=atoi(p);
        assert(childInSen>-1);

        if(deptree[parentInSen].wordInVocab==-2)
            deptree[parentInSen].wordInVocab=parentInVocab;
        else
            assert(deptree[parentInSen].wordInVocab==parentInVocab);

        if(deptree[childInSen].wordInVocab==-2)
            deptree[childInSen].wordInVocab=childInVocab;
        else
            assert(deptree[childInSen].wordInVocab==childInVocab);

        deptree[childInSen].parent=parentInSen;
        deptree[parentInSen].child.push_back(childInSen);

    }

    assert(!feof(fin));
    fgets(temp,MAX_STRING,fin);
    assert(!strcmp(temp,"\n"));
}

int DepTree::GetWordInPos(int pos) {
    return deptree[pos].wordInVocab;
}

int DepTree::GetSenlen() {
    return senlen;
}

int DepTree::GetWordCount() {
    return wordCount;
}

vector<int> DepTree::GetSample(int pos,int window){
    vector<int> sam;
    if(window==0)
        return sam;
    int p=deptree[pos].parent;

    //to only parent
    if(p!=0) {
        for (int j = 0; j < window; j++) {//down to top sample
            if (p == 0)
                break;
            sam.push_back(p);
            p = deptree[p].parent;
        }
    }

    if(deptree[pos].child.empty())
        return sam;

    queue<int> q;
    //to child
    for(int j=0;j<deptree[pos].child.size();j++){
        q.push(deptree[pos].child[j]);
    }

    for(int j=0;j<window;j++){
        if(!q.empty()) {
            int qsize = q.size();
            for (int k = 0; k < qsize; k++) {
                int t = q.front();
                sam.push_back(t);
                q.pop();
                if (deptree[t].child.empty())
                    continue;
                for (int i = 0; i < deptree[t].child.size(); i++) {
                    q.push(deptree[t].child[i]);
                }
            }
        }
    }
    return sam;
}
