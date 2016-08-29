//
// Created by bruce on 7/13/16.
//

#include "DepSkgNeg.h"

DepSkgNeg::DepSkgNeg(const Vocab& v):table_size((int)1e8),vocab(v){//initialize default parameter
    alpha=0.025;
    starting_alpha=0.025;
    binary=0;
    iter=5;
    file_size=0;
    bigdata=0;
    debug_mode=2;
    layer1_size=200;
    word_count_total=0;
    tree_count_total=0;
    train_file[0]=0;
    syn0Word=NULL;
    syn1Word=NULL;
    syn1WordPos=NULL;
    syn1WordPosRel=NULL;
    syn1RelWordPos=NULL;
    expTable = (real *)malloc((EXP_TABLE_SIZE + 1) * sizeof(real));
    if (expTable == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    for (int i = 0; i < EXP_TABLE_SIZE; i++) {
        expTable[i] = (real)exp((i / (real)EXP_TABLE_SIZE * 2 - 1) * MAX_EXP); // Precompute the exp() table
        expTable[i] = expTable[i] / (expTable[i] + 1);                   // Precompute f(x) = x / (x + 1)
    }
    table=NULL;
    sample=1e-4;
    negative=5;
    num_threads=8;

    //InitNet
    long long a, b;
    unsigned long long next_random = 1;

    unsigned int vocabWord_size=vocab.GetVocabWordSize();
    posix_memalign((void **)&syn0Word, 128,  vocabWord_size * layer1_size * sizeof(real));
    posix_memalign((void **)&syn1Word, 128,  vocabWord_size * layer1_size * sizeof(real));

    unsigned int vocabWordPos_size=vocab.GetVocabWordPosSize();
    posix_memalign((void **)&syn1WordPos, 128,  vocabWordPos_size * layer1_size * sizeof(real));

    unsigned int vocabWordPosRel_size=vocab.GetVocabWordPosRelSize();
    posix_memalign((void **)&syn1WordPosRel, 128,  vocabWordPosRel_size * layer1_size * sizeof(real));

    unsigned int vocabRelWordPos_size=vocab.GetVocabRelWordPosSize();
    posix_memalign((void **)&syn1RelWordPos, 128,  vocabRelWordPos_size * layer1_size * sizeof(real));

    if (syn0Word == NULL || syn1Word == NULL || syn1WordPos==NULL || syn1RelWordPos==NULL || syn1WordPosRel==NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    for (a = 0; a < vocabWord_size; a++)
        for (b = 0; b < layer1_size; b++) {
            next_random = next_random * (unsigned long long)25214903917 + 11;
            syn0Word[a * layer1_size + b] = (real)((((next_random & 0xFFFF) / (real)65536) - 0.5) / layer1_size);
            next_random = next_random * (unsigned long long)25214903917 + 11;
            syn1Word[a * layer1_size + b] = (real)((((next_random & 0xFFFF) / (real)65536) - 0.5) / layer1_size);
        }

    for (a = 0; a < vocabWordPos_size; a++)
        for (b = 0; b < layer1_size; b++) {
            next_random = next_random * (unsigned long long)25214903917 + 11;
            syn1WordPos[a * layer1_size + b] = (real)((((next_random & 0xFFFF) / (real)65536) - 0.5) / layer1_size);
        }

    for (a = 0; a < vocabWordPosRel_size; a++)
        for (b = 0; b < layer1_size; b++) {
            next_random = next_random * (unsigned long long)25214903917 + 11;
            syn1WordPosRel[a * layer1_size + b] = (real)((((next_random & 0xFFFF) / (real)65536) - 0.5) / layer1_size);
        }

    for (a = 0; a < vocabRelWordPos_size; a++)
        for (b = 0; b < layer1_size; b++) {
            next_random = next_random * (unsigned long long)25214903917 + 11;
            syn1RelWordPos[a * layer1_size + b] = (real)((((next_random & 0xFFFF) / (real)65536) - 0.5) / layer1_size);
        }
}

//Set parameter from user configuration
void DepSkgNeg::Setlayer1_size(unsigned int x){
    layer1_size=x;
}

void DepSkgNeg::SetBigdata(int x){
    bigdata=x;
}

void DepSkgNeg::SetTrainfile(const char *f) {
    strcpy(train_file,f);
}

void DepSkgNeg::SetDebugmode(int x) {
    debug_mode=x;
}

void DepSkgNeg::SetBinary(int x) {
    binary=x;
}
void DepSkgNeg::SetSample(real x){
    sample=x;
}

void DepSkgNeg::SetAlpha(real x){
    alpha=x;
}

void DepSkgNeg::SetNumthread(int x){
    num_threads=x;
}

void DepSkgNeg::SetNegative(int x){
    negative=x;
}

void DepSkgNeg::SetIter(int x) {
    iter=x;
}

void DepSkgNeg::InitUnigramTable() {// initialize a table for negative sampling
    unsigned int a, i;
    double train_words_pow = 0;
    double d1, power = 0.75;
    table = (int *)malloc(table_size * sizeof(int));
    for (a = 0; a < vocab.GetVocabWordSize(); a++) train_words_pow += pow(vocab.GetVocabWordCn(a), power);
    i = 0;
    d1 = pow(vocab.GetVocabWordCn(i), power) / train_words_pow;
    for (a = 0; a < table_size; a++) {
        table[a] = i;
        if (a / (double)table_size > d1) {
            i++;
            d1 += pow(vocab.GetVocabWordCn(i), power) / train_words_pow;
        }
        if (i >= vocab.GetVocabWordSize()) i = vocab.GetVocabWordSize() - 1;
    }
}

void* DepSkgNeg::BasicTrainModelThread(void *param){// multithread basic
    Para *p=(Para*)param;
    DepSkgNeg* pS=(DepSkgNeg*)((*p).pSelf);
    pS->TrainModelThread((*p).id);
    pthread_exit(NULL);
}

void DepSkgNeg::FindTreeStart(FILE *f) {
    char line[MAX_STRING];
    //find a empty lines as the start of a tree
    while(!feof(f)){
        fgets(line,MAX_STRING,f);
        if(!strcmp(line,"\n")) {
            break;
        }
    }
}

void DepSkgNeg::TrainModelThread(int id){
    int sentence_length = 0;
    unsigned int word_count=0,last_word_count=0,tree_count=0,last_tree_count=0;
    int local_iter=iter;
    unsigned long long next_random = (unsigned long long)id;
    clock_t now;
    unsigned int vocab_size = vocab.GetVocabWordSize();

    real *neu1e = (real *)calloc(layer1_size, sizeof(real));
    FILE *fi = fopen(train_file, "rb");
    if (fi == NULL) {
        fprintf(stderr, "no such file or directory: %s", train_file);
        exit(1);
    }

    fseek(fi, file_size / (long long)num_threads * id, SEEK_SET);
    FindTreeStart(fi);

    DepTree depTree;

    long long total_words=vocab.GetTotalWords();
    long long total_trees=vocab.GetTrainTrees();

    while (true) {
        if (word_count - last_word_count >= 10000) {  //update the learning rate for every 100 trees
            tree_count_total += tree_count - last_tree_count;  //global count of trees
            word_count_total  += word_count - last_word_count;
            last_word_count = word_count;
            last_tree_count = tree_count;
            if ((debug_mode > 1)) {
                now = clock();
                printf("%cAlpha: %f  Word Progress: %.2f%% Tree Progress: %.2f%%  Trees/thread/sec: %.2fk  Words/thread/sec: %.2fk ", 13,
                       alpha,
                       word_count_total / (real) (iter * total_words + 1) * 100,
                       tree_count_total / (real) (iter * total_trees + 1) * 100,
                       tree_count_total / ((real) (now - start + 1) / (real) CLOCKS_PER_SEC * 1000),
                       word_count_total / ((real) (now - start + 1) / (real) CLOCKS_PER_SEC * 1000));
                fflush(stdout);
            }
            alpha = starting_alpha * (1 - word_count_total /(real) (iter * total_words + 1));
            if (alpha < starting_alpha * 0.0001) alpha = starting_alpha * (real)0.0001;

        }

        //Step1: load a tree into mem
        depTree.GetDepTreeFromFilePointer(fi);
        sentence_length = depTree.senlen;

        if(sentence_length==-1 && feof(fi)){
            tree_count_total += tree_count - last_tree_count;
            word_count_total += word_count - last_word_count;
            local_iter--;// next iteration
            if (local_iter == 0)
                break;
            tree_count = 0;
            word_count=0;
            last_tree_count = 0;
            last_word_count = 0;
            fseek(fi, file_size / (long long) num_threads * id, SEEK_SET);
            FindTreeStart(fi);
            continue;
        }

        //step2:train a tree
        for (unsigned int sentence_position = 1; sentence_position <= sentence_length; sentence_position++) {

            const TreeNode *cur = &depTree.deptree[sentence_position];

            char wordPos[MAX_STRING];
            strcpy(wordPos, cur->wordPos);

            //to parent negative sampling
            int parent = cur->parent;
            int parentInVocab = -1;
            if (parent != 0) {
                const TreeNode *curParent = &depTree.deptree[parent];

                char parentWord[MAX_STRING];
                strcpy(parentWord, curParent->wordPos);
                for (int i = 0; i < strlen(parentWord); i++)
                    if (parentWord[i] == '/') {
                        parentWord[i] = 0;
                        break;
                    }
                //parent is target word
                parentInVocab = vocab.SearchVocabWord(parentWord);

                if (parentInVocab != -1) {
                    bool update = true;
                    char relWordPos[MAX_STRING];
                    strcpy(relWordPos, cur->toRel);

                    long lrel = strlen(relWordPos);
                    relWordPos[lrel] = '/';
                    relWordPos[lrel + 1] = 0;
                    strcat(relWordPos, wordPos);

                    int p_relWordPos = vocab.SearchVocabRelWordPos(relWordPos);
                    if (p_relWordPos != -1) {
                        int l1 = p_relWordPos * layer1_size;
                        int label, target;
                        for (unsigned int c = 0; c < layer1_size; c++) neu1e[c] = 0;
                        for (int d = 0; d < negative + 1; d++) {
                            if (d == 0) {
                                target = parentInVocab;
                                label = 1;
                            }
                            else {
                                next_random = next_random * (unsigned long long) 25214903917 + 11;
                                target = table[(next_random >> 16) % table_size];
                                if (target == 0) target = (int) next_random % (vocab_size - 1) + 1;
                                while (target == parentInVocab) {
                                    next_random = next_random * (unsigned long long) 25214903917 + 11;
                                    target = table[(next_random >> 16) % table_size];
                                    if (target == 0) target = (int) next_random % (vocab_size - 1) + 1;
                                }
                                label = 0;
                            }
                            int l2 = target * layer1_size;
                            real f = 0, g = 0;
                            for (unsigned int c = 0; c < layer1_size; c++)
                                f += syn1RelWordPos[c + l1] * syn0Word[c + l2];
                            if (f > MAX_EXP) g = (label - 1) * alpha;
                            else if (f < -MAX_EXP) g = (label - 0) * alpha;
                            else g = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                            for (unsigned int c = 0; c < layer1_size; c++) neu1e[c] += g * syn0Word[c + l2];
                            if (update) {
                                for (unsigned int c = 0; c < layer1_size; c++)
                                    syn0Word[c + l2] += g * syn1RelWordPos[c + l1];
                                update = false;
                            }
                        }
                        // Learn weights input -> hidden
                        for (unsigned int c = 0; c < layer1_size; c++) syn1RelWordPos[c + l1] += neu1e[c];
                    }

                    int p_wordPos = vocab.SearchVocabWordPos(wordPos);
                    if (p_wordPos != -1) {
                        int l1 = p_wordPos * layer1_size;
                        int label, target;
                        for (unsigned int c = 0; c < layer1_size; c++) neu1e[c] = 0;
                        for (int d = 0; d < negative + 1; d++) {
                            if (d == 0) {
                                target = parentInVocab;
                                label = 1;
                            }
                            else {
                                next_random = next_random * (unsigned long long) 25214903917 + 11;
                                target = table[(next_random >> 16) % table_size];
                                if (target == 0) target = (int) next_random % (vocab_size - 1) + 1;
                                while (target == parentInVocab) {
                                    next_random = next_random * (unsigned long long) 25214903917 + 11;
                                    target = table[(next_random >> 16) % table_size];
                                    if (target == 0) target = (int) next_random % (vocab_size - 1) + 1;
                                }
                                label = 0;
                            }
                            int l2 = target * layer1_size;
                            real f = 0, g = 0;
                            for (unsigned int c = 0; c < layer1_size; c++) f += syn1WordPos[c + l1] * syn0Word[c + l2];
                            if (f > MAX_EXP) g = (label - 1) * alpha;
                            else if (f < -MAX_EXP) g = (label - 0) * alpha;
                            else g = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                            for (unsigned int c = 0; c < layer1_size; c++) neu1e[c] += g * syn0Word[c + l2];
                            if (update) {
                                for (unsigned int c = 0; c < layer1_size; c++)
                                    syn0Word[c + l2] += g * syn1WordPos[c + l1];
                                update = false;
                            }
                        }
                        // Learn weights input -> hidden
                        for (unsigned int c = 0; c < layer1_size; c++) syn1WordPos[c + l1] += neu1e[c];
                    }

                    int k = 0;
                    while (true) {
                        if (wordPos[k] == '/') {
                            wordPos[k] = 0;
                            break;
                        } else
                            k++;
                    }

                    int p_word = vocab.SearchVocabWord(wordPos);
                    wordPos[k] = '/';//keep wordPos don 't change
                    if (p_word != -1) {
                        int l1 = p_word * layer1_size;
                        int label, target;
                        for (unsigned int c = 0; c < layer1_size; c++) neu1e[c] = 0;
                        for (int d = 0; d < negative + 1; d++) {
                            if (d == 0) {
                                target = parentInVocab;
                                label = 1;
                            }
                            else {
                                next_random = next_random * (unsigned long long) 25214903917 + 11;
                                target = table[(next_random >> 16) % table_size];
                                if (target == 0) target = (int) next_random % (vocab_size - 1) + 1;
                                while (target == parentInVocab) {
                                    next_random = next_random * (unsigned long long) 25214903917 + 11;
                                    target = table[(next_random >> 16) % table_size];
                                    if (target == 0) target = (int) next_random % (vocab_size - 1) + 1;
                                }
                                label = 0;
                            }
                            int l2 = target * layer1_size;
                            real f = 0, g = 0;
                            for (unsigned int c = 0; c < layer1_size; c++) f += syn1Word[c + l1] * syn0Word[c + l2];
                            if (f > MAX_EXP) g = (label - 1) * alpha;
                            else if (f < -MAX_EXP) g = (label - 0) * alpha;
                            else g = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                            for (unsigned int c = 0; c < layer1_size; c++) neu1e[c] += g * syn0Word[c + l2];
                            if (update) {
                                for (unsigned int c = 0; c < layer1_size; c++) syn0Word[c + l2] += g * syn1Word[c + l1];
                                update = false;
                            }
                        }
                        // Learn weights input -> hidden
                        for (unsigned int c = 0; c < layer1_size; c++) syn1Word[c + l1] += neu1e[c];
                    }
                }
            }


            //to children negative sampling
            char childWordPos[MAX_STRING];
            int childInVocab[MAX_SENTENCE_LENGTH];
            int childInSentence[MAX_SENTENCE_LENGTH];
            long childCount = 0;
            if (!cur->child.empty())
                for (int k = 0; k < cur->child.size(); k++) {
                    childInSentence[childCount] = cur->child[k];
                    strcpy(childWordPos, depTree.deptree[childInSentence[childCount]].wordPos);
                    int h = 0;
                    while (true) {
                        if (childWordPos[h] == '/') {
                            childWordPos[h] = 0;
                            break;
                        } else
                            h++;
                    }
                    childInVocab[childCount] = vocab.SearchVocabWord(childWordPos);
                    if (childInVocab[childCount] != -1)
                        childCount++;
                }

            if (childCount != 0)
                for (int j = 0; j < childCount; j++) {//child is target word
                    bool update = true;
                    char wordPosRel[MAX_STRING];
                    strcpy(wordPosRel, wordPos);

                    long l = strlen(wordPosRel);
                    wordPosRel[l] = '/';
                    wordPosRel[l + 1] = 0;
                    strcat(wordPosRel, depTree.deptree[childInSentence[j]].toRel);

                    int c_wordPosRel = vocab.SearchVocabWordPosRel(wordPosRel);
                    if (c_wordPosRel != -1) {
                        int l1 = c_wordPosRel * layer1_size;
                        int label, target;
                        for (unsigned int c = 0; c < layer1_size; c++) neu1e[c] = 0;
                        for (int d = 0; d < negative + 1; d++) {
                            if (d == 0) {
                                target = childInVocab[j];
                                label = 1;
                            }
                            else {
                                while (true) {
                                    next_random = next_random * (unsigned long long) 25214903917 + 11;
                                    target = table[(next_random >> 16) % table_size];
                                    if (target == 0) target = (int) next_random % (vocab_size - 1) + 1;
                                    if (target == parentInVocab) continue;
                                    int flagC = false;
                                    for (int kk = 0; kk < childCount; kk++) {
                                        if (target == childInVocab[kk]) {
                                            flagC = true;
                                            break;
                                        }
                                    }
                                    if (flagC) continue;
                                    break;
                                }
                                label = 0;
                            }
                            int l2 = target * layer1_size;
                            real f = 0, g = 0;
                            for (unsigned int c = 0; c < layer1_size; c++)
                                f += syn1WordPosRel[c + l1] * syn0Word[c + l2];
                            if (f > MAX_EXP) g = (label - 1) * alpha;
                            else if (f < -MAX_EXP) g = (label - 0) * alpha;
                            else g = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                            for (unsigned int c = 0; c < layer1_size; c++) neu1e[c] += g * syn0Word[c + l2];
                            if (update) {
                                for (unsigned int c = 0; c < layer1_size; c++)
                                    syn0Word[c + l2] += g * syn1WordPosRel[c + l1];
                                update = false;
                            }
                        }
                        // Learn weights input -> hidden
                        for (unsigned int c = 0; c < layer1_size; c++) syn1WordPosRel[c + l1] += neu1e[c];

                    }
                    int c_wordPos = vocab.SearchVocabWordPos(wordPos);
                    if (c_wordPos != -1) {
                        int l1 = c_wordPos * layer1_size;
                        int label, target;
                        for (unsigned int c = 0; c < layer1_size; c++) neu1e[c] = 0;
                        for (int d = 0; d < negative + 1; d++) {
                            if (d == 0) {
                                target = childInVocab[j];
                                label = 1;
                            }
                            else {
                                while (true) {
                                    next_random = next_random * (unsigned long long) 25214903917 + 11;
                                    target = table[(next_random >> 16) % table_size];
                                    if (target == 0) target = (int) next_random % (vocab_size - 1) + 1;
                                    if (target == parentInVocab) continue;
                                    int flagC = false;
                                    for (int kk = 0; kk < childCount; kk++) {
                                        if (target == childInVocab[kk]) {
                                            flagC = true;
                                            break;
                                        }
                                    }
                                    if (flagC) continue;
                                    break;
                                }
                                label = 0;
                            }
                            int l2 = target * layer1_size;
                            real f = 0, g = 0;
                            for (unsigned int c = 0; c < layer1_size; c++) f += syn1WordPos[c + l1] * syn0Word[c + l2];
                            if (f > MAX_EXP) g = (label - 1) * alpha;
                            else if (f < -MAX_EXP) g = (label - 0) * alpha;
                            else g = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                            for (unsigned int c = 0; c < layer1_size; c++) neu1e[c] += g * syn0Word[c + l2];
                            if (update) {
                                for (unsigned int c = 0; c < layer1_size; c++)
                                    syn0Word[c + l2] += g * syn1WordPos[c + l1];
                                update = false;
                            }
                        }
                        // Learn weights input -> hidden
                        for (unsigned int c = 0; c < layer1_size; c++) syn1WordPos[c + l1] += neu1e[c];
                    }
                    int k = 0;
                    while (true) {
                        if (wordPos[k] == '/') {
                            wordPos[k] = 0;
                            break;
                        } else
                            k++;
                    }

                    int c_word = vocab.SearchVocabWord(wordPos);
                    wordPos[k] = '/';//keep wordPos don 't change
                    if (c_word != -1) {
                        int l1 = c_word * layer1_size;
                        int label, target;
                        for (unsigned int c = 0; c < layer1_size; c++) neu1e[c] = 0;
                        for (int d = 0; d < negative + 1; d++) {
                            if (d == 0) {
                                target = childInVocab[j];
                                label = 1;
                            }
                            else {
                                while (true) {
                                    next_random = next_random * (unsigned long long) 25214903917 + 11;
                                    target = table[(next_random >> 16) % table_size];
                                    if (target == 0) target = (int) next_random % (vocab_size - 1) + 1;
                                    if (target == parentInVocab) continue;
                                    int flagC = false;
                                    for (int kk = 0; kk < childCount; kk++) {
                                        if (target == childInVocab[kk]) {
                                            flagC = true;
                                            break;
                                        }
                                    }
                                    if (flagC) continue;
                                    break;
                                }
                                label = 0;
                            }
                            int l2 = target * layer1_size;
                            real f = 0, g = 0;
                            for (unsigned int c = 0; c < layer1_size; c++) f += syn1Word[c + l1] * syn0Word[c + l2];
                            if (f > MAX_EXP) g = (label - 1) * alpha;
                            else if (f < -MAX_EXP) g = (label - 0) * alpha;
                            else g = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                            for (unsigned int c = 0; c < layer1_size; c++) neu1e[c] += g * syn0Word[c + l2];
                            if (update) {
                                for (unsigned int c = 0; c < layer1_size; c++) syn0Word[c + l2] += g * syn1Word[c + l1];
                                update = false;
                            }
                        }
                        // Learn weights input -> hidden
                        for (unsigned int c = 0; c < layer1_size; c++) syn1Word[c + l1] += neu1e[c];
                    }
                }
        }
        tree_count++;
        word_count+=sentence_length;

        if(word_count > total_words/num_threads){// every thread can train trees ,not more than train_trees/num_threads
            tree_count_total += tree_count - last_tree_count;
            word_count_total += word_count - last_word_count;
            local_iter--;// next iteration
            if(id==0 && bigdata==1){
                char temp[MAX_STRING];
                sprintf(temp,"%d",iter-local_iter);
                SaveWordVectors(temp);
            }
            if (local_iter == 0)
                break;
            tree_count = 0;
            word_count=0;
            last_tree_count = 0;
            last_word_count = 0;
            fseek(fi, file_size / (long long) num_threads * id, SEEK_SET);
            FindTreeStart(fi);
            continue;
        }
    }
    fclose(fi);
    free(neu1e);

}


void DepSkgNeg::TrainModel() {
    starting_alpha = alpha;
    FILE *fin = fopen(train_file, "rb");
    if (fin == NULL) {
        printf("ERROR: training data file not found!\n");
        exit(1);
    }
    fseek(fin, 0, SEEK_END);
    file_size = ftell(fin);
    fclose(fin);
    pthread_t *pt = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    if (pt == NULL) {
        fprintf(stderr, "cannot allocate memory for threads\n");
        exit(1);
    }

    printf("Starting training using file %s\n", train_file);

    start = clock();
    InitUnigramTable();

    Para param[num_threads];
    for (int a = 0; a < num_threads; a++) {
        param[a].pSelf=this;
        param[a].id=a;
    }

    for (int a = 0; a < num_threads; a++) pthread_create(&pt[a], NULL, BasicTrainModelThread,&param[a]);
    for (int a = 0; a < num_threads; a++) pthread_join(pt[a], NULL);

    printf("\nTrees trained: %u\n", tree_count_total/iter);
    printf("Words trained: %u\n", word_count_total/iter);

    free(pt);

}

void DepSkgNeg::SaveWordVectors(const char *output_file) {
    FILE *fo = fopen(output_file, "wb");
    if (fo == NULL) {
        fprintf(stderr, "Cannot open %s: permission denied\n", output_file);
        exit(1);
    }
    unsigned int a=0,b=0;
    // Save the word vectors
    unsigned int vocab_size=vocab.GetVocabWordSize();
    fprintf(fo, "%u %u\n", vocab_size, layer1_size);
    for (a = 0; a < vocab_size; a++) {
        const char* wordOfa=vocab.GetVocabWord(a);
        if ( wordOfa!= NULL) {
            fprintf(fo,"%s ",wordOfa);
        }
        if (binary) for (b = 0; b < layer1_size; b++) fwrite(&syn0Word[a * layer1_size + b], sizeof(real), 1, fo);
        else for (b = 0; b < layer1_size; b++) fprintf(fo, "%lf ", syn0Word[a * layer1_size + b]);
        fprintf(fo, "\n");
    }
}

