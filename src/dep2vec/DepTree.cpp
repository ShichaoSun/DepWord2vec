//
// Created by bruce on 7/13/16.
//
#include "DepTree.h"

DepTree::DepTree(){
    senlen=-1;
    for (int i = 0; i <= MAX_SENTENCE_LENGTH; i++) {
        deptree[i].parent = -1;
        deptree[i].wordPos[0] = 0;
    }
}

void DepTree::ClearDepTree() {//clear the tree and initialization
    if(senlen>=0) {
        for (int i = 0; i <= senlen; i++) {
            deptree[i].parent = -1;
            deptree[i].wordPos[0] = 0;
            deptree[i].toRel[0]=0;
            deptree[i].child.clear();//clear the vector
        }
    }
    senlen=-1;
}

int DepTree::GetDepTreeFromFilePointer(FILE *fin) {// from the file pointer generate a dependency tree
    ClearDepTree();  //clear
    char temp[MAX_STRING],rel[MAX_STRING],parentw[MAX_STRING],childw[MAX_STRING];
    char *p;
    const char *d = " ";
    while (true) {
        if (feof(fin))
            return 0;
        fgets(temp, MAX_STRING, fin);  //read in a line
        if (strlen(temp) < 2)// isn't a number
            continue;
        int j = 0;
        for (j = 0; j < strlen(temp) - 1; j++)// is a number
            if (!isdigit(temp[j]))
                break;
        if (j == strlen(temp) - 1 && temp[j] == '\n')
            break;
        else
            continue;
    }
    senlen = atoi(temp);
    assert(senlen > 0 && senlen + 1 < MAX_SENTENCE_LENGTH);

    for (int i = 0; i < senlen; i++) {//loop the tree node
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

        p = strsep(&q, d);   //child position in sentence
        assert(strlen(p) > 0);
        int childInSen = atoi(p);
        assert(childInSen > -1);

        deptree[childInSen].parent = parentInSen;
        deptree[parentInSen].child.push_back(childInSen);

        if(deptree[childInSen].toRel[0]==0)
            strcpy(deptree[childInSen].toRel,rel);
        if(strcmp(deptree[childInSen].toRel,rel)){
            GetDepTreeFromFilePointer(fin);
            return 0;
        }

        if(deptree[childInSen].wordPos[0]==0)
            strcpy(deptree[childInSen].wordPos,childw);
        if(strcmp(deptree[childInSen].wordPos,childw)){
            GetDepTreeFromFilePointer(fin);
            return 0;
        }

        if(deptree[parentInSen].wordPos[0]==0)
            strcpy(deptree[parentInSen].wordPos,parentw);
        if(strcmp(deptree[parentInSen].wordPos,parentw)){
            GetDepTreeFromFilePointer(fin);
            return 0;
        }
    }

    fgets(temp, MAX_STRING, fin);
    if(strcmp(temp, "\n")) {// is a good tree
        GetDepTreeFromFilePointer(fin);
        return 0;
    }
    return 0;
}
