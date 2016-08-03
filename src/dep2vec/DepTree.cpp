//
// Created by bruce on 7/13/16.
//
#include "DepTree.h"

DepTree::DepTree(const Vocab &v):vocab(v){
    wordCountActual=0;
    senlen=-1;
    for (int i = 0; i <= MAX_SENTENCE_LENGTH; i++) {
        deptree[i].parent = -1;
        deptree[i].wordInVocab = -2;
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
    wordCountActual=0;
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

    wordCountActual=senlen;

    for(int i=0;i<senlen;i++){
        assert(!feof(fin));
        fgets(temp,MAX_STRING,fin);

        char *q=temp;

        p = strsep(&q, d);  //dep_relationship
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

        if(deptree[childInSen].wordInVocab==-2) //-2: uninitialized ; -1: not in dictionary
            deptree[childInSen].wordInVocab=childInVocab;
        else
            assert(deptree[childInSen].wordInVocab==childInVocab);

        if(childInVocab==-1)  // -1: not in dictionary,and it is not trained
            wordCountActual--;

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

int DepTree::GetWordCountActual() {
    return wordCountActual;
}

int DepTree::GetSenlen() {
    return senlen;
}

int DepTree::SubSampling(int pos, int *visited, queue<int> &q, vector<int> &sam, real sample, unsigned long long &next_random){// frequent words will be seen empty,and use its parent and children replacing its positon
    if(pos==0 || visited[pos])
        return 0;

    visited[pos]=1;

    long long word=deptree[pos].wordInVocab;
    if(word==-1){
        q.push(pos);
        return 0;
    }

    long long train_words=vocab.GetTrainWords();

    real ran = (sqrt(vocab.GetVocabWordCn(word) / (sample * train_words)) + 1) * (sample * train_words) / vocab.GetVocabWordCn(word);
    //real ran = (sqrt(vocab[word].cn / (sample * train_words)) + 1) * (sample * train_words) / vocab[word].cn;
    next_random = next_random * (unsigned long long) 25214903917 + 11;

    if (ran < (next_random & 0xFFFF) / (real) 65536){
        SubSampling(deptree[pos].parent,visited,q,sam,sample,next_random);
        if(!deptree[pos].child.empty())
            for(int i=0;i<deptree[pos].child.size();i++)
                SubSampling(deptree[pos].child[i],visited,q,sam,sample,next_random);
    }else{
        q.push(pos);
        sam.push_back(pos);
    }

    return 0;
}

vector<int> DepTree::GetSample(int pos, int window, real sample, unsigned long long &next_random){// get sample
    vector<int> sam;
    queue<int> q;
    if(window==0)
        return sam;

    int visited[MAX_SENTENCE_LENGTH+1];
    for(int i=0;i<MAX_SENTENCE_LENGTH+1;i++) visited[i]=0;

    visited[pos]=1;
    q.push(pos);

    for(int i=0;i<window;i++) {// BFS , window times
        unsigned long ql = q.size();
        if (ql==0) break;
        for (int j = 0; j < ql; j++) {
            int cur = q.front();
            q.pop();

            //to parent
            int p = deptree[cur].parent;
            if (p != 0 && visited[p] != 1) {
                //The subsampling randomly discards frequent words while keeping the ranking same
                if (sample > 0) {
                    SubSampling(p, visited, q, sam, sample, next_random);
                } else {
                    visited[p] = 1;
                    if(deptree[p].wordInVocab!=-1)
                        sam.push_back(p);
                    q.push(p);
                }

            }

            //to children
            if (!deptree[cur].child.empty())
                for (int k = 0; k < deptree[cur].child.size(); k++) {
                    int c = deptree[cur].child[k];
                    if(sample>0){
                        SubSampling(c,visited, q, sam, sample, next_random);
                    }else {
                        if (visited[c] != 1) {
                            visited[c]=1;
                            if(deptree[c].wordInVocab!=-1)
                                sam.push_back(c);
                            q.push(c);
                        }
                    }
                }
        }

    }

    return sam;
}
