//
// Created by bruce on 7/13/16.
//
#include "DepTree.h"

DepTree::DepTree(const Vocab &v):vocab(v){
    senlen=-1;
    for (int i = 0; i <= MAX_SENTENCE_LENGTH; i++) {
        deptree[i].parent = -1;
        deptree[i].wordInVocab = -2;
        //deptree[i].child.clear();
    }
}

void DepTree::ClearDepTree() {
    if(senlen>=0) {
        for (int i = 0; i <= senlen; i++) {
            deptree[i].parent = -1;
            deptree[i].wordInVocab = -2;
            deptree[i].child.clear();
        }
    }
    senlen=-1;
}

void DepTree::GetDepTreeFromFilePointer(FILE *fin){
    ClearDepTree();  //clear
    char temp[MAX_STRING];
    char *p;
    const char *d=" ";
    while (true){
        if(feof(fin))
            return;
        fgets(temp,MAX_STRING,fin);  //read in a line
        if(strlen(temp)<2)
            continue;
        int j=0;
        for(j=0;j<strlen(temp)-1;j++)
            if(!isdigit(temp[j]))
                break;
        if(j==strlen(temp)-1 && temp[j]=='\n')
            break;
        else{
            fprintf(stderr,"don't find the size of tree");
            exit(1);
        }
    }
    senlen=atoi(temp);
    assert(senlen>0 && senlen+1 < MAX_SENTENCE_LENGTH);

    for(int i=0;i<senlen;i++){
        assert(!feof(fin));
        fgets(temp,MAX_STRING,fin);

        char *q=temp;

        p = strsep(&q, d);  //dep_rel
        assert(strlen(p)>0);

        p = strsep(&q, d);  //parent word
        assert(strlen(p)>0);
        int parentInVocab=vocab.SearchVocab(p);  //parent word id

        p = strsep(&q, d);  //parent position in sentence
        assert(strlen(p)>0);
        int parentInSen=atoi(p);
        assert(parentInSen>-1);

        p = strsep(&q, d);  //child word
        assert(strlen(p)>0);
        int childInVocab=vocab.SearchVocab(p);

        p = strsep(&q, d);   //child position in sentence
        assert(strlen(p)>0);
        int childInSen=atoi(p);
        assert(childInSen>-1);

        if(deptree[parentInSen].wordInVocab==-2)   //-2: uninitialized ; -1: not in dictionary
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

vector<int> DepTree::GetSample(int pos,int window){
    vector<int> sam;
    queue<int> q;
    if(window==0)
        return sam;
    q.push(pos);

    int visited[senlen+1];
    memset(visited,0,senlen+1);

    for(int i=0;i<=window;i++){
        unsigned long ql=q.size();
        if(ql!=0) {
            for (int j = 0; j < ql; j++) {
                int cur = q.front();
                q.pop();
                visited[cur]=1;
                int p=deptree[cur].parent;
                if(p!=0 && visited[p]!=1)
                    q.push(deptree[cur].parent);
                if(!deptree[cur].child.empty())
                    for(int k=0;k<deptree[cur].child.size();k++) {
                        int c=deptree[cur].child[k];
                        if (visited[c]!=1)
                            q.push(c);
                    }
            }
        }else
            break;
    }
    for(int i=0;i<senlen+1;i++)
        if(visited[i] && i!=pos)
            sam.push_back(i);
    return sam;
}
