//
// Created by bruce on 7/13/16.
//
#include "DepTree.h"

DepTree::DepTree(const Vocab &v):vocab(v){
    wordCountActual=0;
    senlen=-1;
    for (int i = 0; i <= MAX_SENTENCE_LENGTH; i++) {
        deptree[i].parent = -1;
        deptree[i].wordPos[0] = 0;
    }
}

void DepTree::ClearDepTree() {
    if(senlen>=0) {
        for (int i = 0; i <= senlen; i++) {
            deptree[i].parent = -1;
            deptree[i].wordPos[0] = 0;
            deptree[i].child.clear();
        }
    }
    senlen=-1;
    wordCountActual=0;
}

void DepTree::GetDepTreeFromFilePointer(FILE *fin) {
    ClearDepTree();  //clear
    char temp[MAX_STRING],rel[MAX_STRING],parentw[MAX_STRING],childw[MAX_STRING];
    char *p;
    const char *d = " ";
    while (true) {
        if (feof(fin))
            return;
        fgets(temp, MAX_STRING, fin);  //read in a line
        if (strlen(temp) < 2)
            continue;
        int j = 0;
        for (j = 0; j < strlen(temp) - 1; j++)
            if (!isdigit(temp[j]))
                break;
        if (j == strlen(temp) - 1 && temp[j] == '\n')
            break;
        else
            continue;
    }
    senlen = atoi(temp);
    assert(senlen > 0 && senlen + 1 < MAX_SENTENCE_LENGTH);

    wordCountActual = senlen;

    for (int i = 0; i < senlen; i++) {
        assert(!feof(fin));
        fgets(temp, MAX_STRING, fin);

        char *q = temp;

        p = strsep(&q, d);  //dep_relationship
        assert(strlen(p) > 0);
        strcpy(rel, p);

        p = strsep(&q, d);  //parent word
        assert(strlen(p) > 0);
        strcpy(parentw,p);

        p = strsep(&q, d);  //parent position in sentence
        assert(strlen(p) > 0);
        int parentInSen = atoi(p);
        assert(parentInSen > -1);

        p = strsep(&q, d);  //child word
        assert(strlen(p) > 0);
        strcpy(childw, p);
        for (int k = 0; k < strlen(childw); k++)
            if (childw[k] == '/') {
                childw[k] = 0;
                break;
            }

        p = strsep(&q, d);   //child position in sentence
        assert(strlen(p) > 0);
        int childInSen = atoi(p);
        assert(childInSen > -1);

        deptree[childInSen].parent = parentInSen;
        deptree[parentInSen].child.push_back(childInSen);

        if(deptree[childInSen].toRel[0]==0)
            strcpy(deptree[childInSen].toRel,rel);
        assert(!strcmp(deptree[childInSen].toRel,rel));

        if(deptree[childInSen].wordPos[0]==0)
            strcpy(deptree[childInSen].wordPos,childw);
        assert(!strcmp(deptree[childInSen].wordPos,childw));

        if(deptree[parentInSen].wordPos[0]==0)
            strcpy(deptree[parentInSen].wordPos,parentw);
        assert(!strcmp(deptree[parentInSen].wordPos,parentw));
    }

    fgets(temp, MAX_STRING, fin);
    assert(!strcmp(temp, "\n"));
}
