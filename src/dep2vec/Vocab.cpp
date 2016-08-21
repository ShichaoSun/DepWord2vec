//
// Created by bruce on 7/13/16.
//

#include "Vocab.h"

Vocab::Vocab():vocab_hash_size(30000000){
    vocabWord_max_size=1000;
    vocabWordPos_max_size=1000;
    vocabWordPosRel_max_size=1000;
    vocabRelWordPos_max_size=1000;

    train_words=0;
    train_trees=0;
    total_words=0;

    min_count=5;
    min_reduce=1;

    vocabWord_size=0;
    vocabWordPos_size=0;
    vocabWordPosRel_size=0;
    vocabRelWordPos_size=0;

    vocabWord=(vocab_cell*)calloc(vocabWord_max_size,sizeof(vocab_cell));
    vocabWordPos=(vocab_cell*)calloc(vocabWordPos_max_size,sizeof(vocab_cell));
    vocabWordPosRel=(vocab_cell*)calloc(vocabWordPosRel_max_size,sizeof(vocab_cell));
    vocabRelWordPos=(vocab_cell*)calloc(vocabRelWordPos_max_size,sizeof(vocab_cell));

    vocabWord_hash=(int*)calloc(vocab_hash_size,sizeof(int));
    vocabWordPos_hash=(int*)calloc(vocab_hash_size,sizeof(int));
    vocabWordPosRel_hash=(int*)calloc(vocab_hash_size,sizeof(int));
    vocabRelWordPos_hash=(int*)calloc(vocab_hash_size,sizeof(int));

}

int Vocab::GetWordHash(const char *word) const{// Returns hash value of a word
    int a, hash = 0;
    for (a = 0; a < strlen(word); a++) hash = hash * 257 + word[a];
    hash = hash % vocab_hash_size;
    return hash;
}

int Vocab::GetWordPosHash(const char *wordPos) const {// Returns hash value of a word
    int a, hash = 0;
    for (a = 0; a < strlen(wordPos); a++) hash = hash * 257 + wordPos[a];
    hash = hash % vocab_hash_size;
    return hash;
}

int Vocab::GetWordPosRelHash(const char *wordPosRel) const {
    int a, hash = 0;
    for (a = 0; a < strlen(wordPosRel); a++) hash = hash * 257 + wordPosRel[a];
    hash = hash % vocab_hash_size;
    return hash;
}

int Vocab::GetRelWordPosHash(const char *relWordPosRel) const {
    int a, hash = 0;
    for (a = 0; a < strlen(relWordPosRel); a++) hash = hash * 257 + relWordPosRel[a];
    hash = hash % vocab_hash_size;
    return hash;
}

int Vocab::SearchVocabWord(const char *word) const {// Returns position of a word in the vocabulary; if the word is not found, returns -1
    int hash = GetWordHash(word);
    while (1) {
        if (vocabWord_hash[hash] == -1) return -1;
        if (!strcmp(word,vocabWord[vocabWord_hash[hash]].cell)) return vocabWord_hash[hash];
        hash = (hash + 1) % vocab_hash_size;
    }
}

int Vocab::SearchVocabWordPos(const char *wordPos) const {
    int hash = GetWordPosHash(wordPos);
    while (1) {
        if (vocabWordPos_hash[hash] == -1) return -1;
        if (!strcmp(wordPos,vocabWordPos[vocabWordPos_hash[hash]].cell)) return vocabWordPos_hash[hash];
        hash = (hash + 1) % vocab_hash_size;
    }
}

int Vocab::SearchVocabWordPosRel(const char *wordPosRel) const {
    int hash = GetWordPosRelHash(wordPosRel);
    while (1) {
        if (vocabWordPosRel_hash[hash] == -1) return -1;
        if (!strcmp(wordPosRel,vocabWordPosRel[vocabWordPosRel_hash[hash]].cell)) return vocabWordPosRel_hash[hash];
        hash = (hash + 1) % vocab_hash_size;
    }
}

int Vocab::SearchVocabRelWordPos(const char *relWordPos) const {
    int hash = GetRelWordPosHash(relWordPos);
    while (1) {
        if (vocabRelWordPos_hash[hash] == -1) return -1;
        if (!strcmp(relWordPos,vocabRelWordPos[vocabRelWordPos_hash[hash]].cell)) return vocabRelWordPos_hash[hash];
        hash = (hash + 1) % vocab_hash_size;
    }
}

char* Vocab::GetVocabWord(unsigned int a) const {
    return vocabWord[a].cell;
}

char* Vocab::GetVocabWordPos(unsigned int a) const {
    return vocabWordPos[a].cell;
}

char* Vocab::GetVocabWordPosRel(unsigned int a) const {
    return vocabWordPosRel[a].cell;
}

char* Vocab::GetVocabRelWordPos(unsigned int a) const {
    return vocabRelWordPos[a].cell;
}

int Vocab::AddWordToVocab(char *word) {// Adds a word to the vocabulary
    unsigned long length = strlen(word) + 1;
    int hash;
    if (length > MAX_STRING) length = MAX_STRING;

    vocabWord[vocabWord_size].cell = (char *)calloc(length, sizeof(char));
    strcpy(vocabWord[vocabWord_size].cell, word);
    vocabWord[vocabWord_size].cn = 1;
    vocabWord_size++;
    // Reallocate memory if needed
    if (vocabWord_size + 2 >= vocabWord_max_size) {
        vocabWord_max_size += 1000;
        vocabWord = (vocab_cell*)realloc(vocabWord, vocabWord_max_size * sizeof(vocab_cell));
    }
    hash = GetWordHash(word);
    while (vocabWord_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
    vocabWord_hash[hash] = vocabWord_size - 1;
    return vocabWord_size - 1;
}

int Vocab::AddWordPosToVocab(char *wordPos){
    unsigned long length = strlen(wordPos) + 1;
    int hash;
    if (length > MAX_STRING) length = MAX_STRING;

    vocabWordPos[vocabWordPos_size].cell = (char *)calloc(length, sizeof(char));
    strcpy(vocabWordPos[vocabWordPos_size].cell, wordPos);
    vocabWordPos[vocabWordPos_size].cn = 1;
    vocabWordPos_size++;
    // Reallocate memory if needed
    if (vocabWordPos_size + 2 >= vocabWordPos_max_size) {
        vocabWordPos_max_size += 1000;
        vocabWordPos = (vocab_cell*)realloc(vocabWordPos, vocabWordPos_max_size * sizeof(vocab_cell));
    }
    hash = GetWordHash(wordPos);
    while (vocabWordPos_hash[hash] != -1) hash = (hash + 1) % vocabWordPos_size;
    vocabWordPos_hash[hash] = vocabWordPos_size - 1;
    return vocabWordPos_size - 1;
}

int Vocab::AddWordPosRelToVocab(char *wordPosRel){
    unsigned long length = strlen(wordPosRel) + 1;
    int hash;
    if (length > MAX_STRING) length = MAX_STRING;

    vocabWordPosRel[vocabWordPosRel_size].cell = (char *)calloc(length, sizeof(char));
    strcpy(vocabWordPosRel[vocabWordPosRel_size].cell, wordPosRel);
    vocabWordPosRel[vocabWordPosRel_size].cn = 1;
    vocabWordPosRel_size++;
    // Reallocate memory if needed
    if (vocabWordPosRel_size + 2 >= vocabWordPosRel_max_size) {
        vocabWordPosRel_max_size += 1000;
        vocabWordPosRel = (vocab_cell*)realloc(vocabWordPosRel, vocabWordPosRel_max_size * sizeof(vocab_cell));
    }
    hash = GetWordHash(wordPosRel);
    while (vocabWordPosRel_hash[hash] != -1) hash = (hash + 1) % vocabWordPosRel_size;
    vocabWordPosRel_hash[hash] = vocabWordPosRel_size - 1;
    return vocabWordPosRel_size - 1;
}

int Vocab::AddRelWordPosToVocab(char *relWordPos){
    unsigned long length = strlen(relWordPos) + 1;
    int hash;
    if (length > MAX_STRING) length = MAX_STRING;

    vocabRelWordPos[vocabRelWordPos_size].cell = (char *)calloc(length, sizeof(char));
    strcpy(vocabRelWordPos[vocabRelWordPos_size].cell, relWordPos);
    vocabRelWordPos[vocabRelWordPos_size].cn = 1;
    vocabRelWordPos_size++;
    // Reallocate memory if needed
    if (vocabRelWordPos_size + 2 >= vocabRelWordPos_max_size) {
        vocabRelWordPos_max_size += 1000;
        vocabRelWordPos = (vocab_cell*)realloc(vocabRelWordPos, vocabRelWordPos_max_size * sizeof(vocab_cell));
    }
    hash = GetWordHash(relWordPos);
    while (vocabRelWordPos_hash[hash] != -1) hash = (hash + 1) % vocabRelWordPos_size;
    vocabRelWordPos_hash[hash] = vocabRelWordPos_size - 1;
    return vocabRelWordPos_size - 1;
}

static int VocabCompare(const void*a,const void *b){
    return (int)(((vocab_cell*)b)->cn - ((vocab_cell*)a)->cn);
}

unsigned int Vocab::GetVocabWordSize() const{
    return vocabWord_size;
}

unsigned int Vocab::GetVocabWordPosSize() const{
    return vocabWordPos_size;
}

unsigned int Vocab::GetVocabWordPosRelSize() const{
    return vocabWordPosRel_size;
}

unsigned int Vocab::GetVocabRelWordPosSize() const{
    return vocabRelWordPos_size;
}

long long Vocab::GetVocabWordCn(unsigned int i) const {
    return vocabWord[i].cn;
}

long long Vocab::GetVocabWordPosCn(unsigned int i) const {
    return vocabWordPos[i].cn;
}

long long Vocab::GetVocabWordPosRelCn(unsigned int i) const {
    return vocabWordPosRel[i].cn;
}

long long Vocab::GetVocabRelWordPosCn(unsigned int i) const {
    return vocabRelWordPos[i].cn;
}

void Vocab::SetMincount(int x){
    min_count=x;
}

void Vocab::SortVocabWord(){
    int a, size;
    int hash;
    // Sort the vocabulary and keep </s> at the first position
    qsort(&vocabWord[0], vocabWord_size, sizeof(vocab_cell), VocabCompare);
    for (a = 0; a < vocab_hash_size; a++) vocabWord_hash[a] = -1;
    size = vocabWord_size;
    train_words = 0;
    for (a = 0; a < size; a++) {
        // Words occuring less than min_count times will be discarded from the vocab
        if (vocabWord[a].cn < min_count) {
            vocabWord_size--;
            free(vocabWord[a].cell);
            vocabWord[a].cell = NULL;
        } else {
            // Hash will be re-computed, as after the sorting it is not actual
            hash=GetWordHash(vocabWord[a].cell);
            while (vocabWord_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
            vocabWord_hash[hash] = a;
            train_words +=vocabWord[a].cn;
        }
    }
    vocabWord = (vocab_cell*)realloc(vocabWord, (vocabWord_size + 1) * sizeof(vocab_cell));
}

void Vocab::SortVocabWordPos(){
    int a, size;
    int hash;
    // Sort the vocabulary and keep </s> at the first position
    qsort(&vocabWordPos[0], vocabWordPos_size, sizeof(vocab_cell), VocabCompare);
    for (a = 0; a < vocab_hash_size; a++) vocabWordPos_hash[a] = -1;
    size = vocabWordPos_size;
    for (a = 0; a < size; a++) {
        // Words occuring less than min_count times will be discarded from the vocab
        if (vocabWordPos[a].cn < min_count) {
            vocabWordPos_size--;
            free(vocabWordPos[a].cell);
            vocabWordPos[a].cell = NULL;
        } else {
            // Hash will be re-computed, as after the sorting it is not actual
            hash=GetWordHash(vocabWordPos[a].cell);
            while (vocabWordPos_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
            vocabWordPos_hash[hash] = a;
        }
    }
    vocabWordPos = (vocab_cell*)realloc(vocabWordPos, (vocabWordPos_size + 1) * sizeof(vocab_cell));
}

void Vocab::SortVocabWordPosRel(){
    int a, size;
    int hash;
    // Sort the vocabulary and keep </s> at the first position
    qsort(&vocabWordPosRel[0], vocabWordPosRel_size, sizeof(vocab_cell), VocabCompare);
    for (a = 0; a < vocab_hash_size; a++) vocabWordPosRel_hash[a] = -1;
    size = vocabWordPosRel_size;
    for (a = 0; a < size; a++) {
        // Words occuring less than min_count times will be discarded from the vocab
        if (vocabWordPosRel[a].cn < min_count) {
            vocabWordPosRel_size--;
            free(vocabWordPosRel[a].cell);
            vocabWordPosRel[a].cell = NULL;
        } else {
            // Hash will be re-computed, as after the sorting it is not actual
            hash=GetWordHash(vocabWordPosRel[a].cell);
            while (vocabWordPosRel_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
            vocabWordPosRel_hash[hash] = a;
        }
    }
    vocabWordPosRel = (vocab_cell*)realloc(vocabWordPosRel, (vocabWordPosRel_size + 1) * sizeof(vocab_cell));
}

void Vocab::SortVocabRelWordPos(){
    int a, size;
    int hash;
    // Sort the vocabulary and keep </s> at the first position
    qsort(&vocabRelWordPos[0], vocabRelWordPos_size, sizeof(vocab_cell), VocabCompare);
    for (a = 0; a < vocab_hash_size; a++) vocabRelWordPos_hash[a] = -1;
    size = vocabRelWordPos_size;
    for (a = 0; a < size; a++) {
        // Words occuring less than min_count times will be discarded from the vocab
        if (vocabRelWordPos[a].cn < min_count) {
            vocabRelWordPos_size--;
            free(vocabRelWordPos[a].cell);
            vocabRelWordPos[a].cell = NULL;
        } else {
            // Hash will be re-computed, as after the sorting it is not actual
            hash=GetWordHash(vocabRelWordPos[a].cell);
            while (vocabRelWordPos_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
            vocabRelWordPos_hash[hash] = a;
        }
    }
    vocabRelWordPos = (vocab_cell*)realloc(vocabRelWordPos, (vocabRelWordPos_size + 1) * sizeof(vocab_cell));
}

void Vocab::ReduceVocabWord(){
    int a,hash;
    unsigned int b = 0;
    for (a = 0; a < vocabWord_size; a++)
        if (vocabWord[a].cn > min_reduce) {
            vocabWord[b].cn = vocabWord[a].cn;
            vocabWord[b].cell = vocabWord[a].cell;
            b++;
        } else
            free(vocabWord[a].cell);
    vocabWord_size = b;
    for (a = 0; a < vocab_hash_size; a++) vocabWord_hash[a] = -1;
    for (a = 0; a < vocabWord_size; a++) {
        // Hash will be re-computed, as it is not actual
        hash = GetWordHash(vocabWord[a].cell);
        while (vocabWord_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
        vocabWord_hash[hash] = a;
    }
    min_reduce++;
}

void Vocab::ReduceVocabWordPos(){
    int a,hash;
    unsigned int b = 0;
    for (a = 0; a < vocabWordPos_size; a++)
        if (vocabWordPos[a].cn > min_reduce) {
            vocabWordPos[b].cn = vocabWordPos[a].cn;
            vocabWordPos[b].cell = vocabWordPos[a].cell;
            b++;
        } else
            free(vocabWordPos[a].cell);
    vocabWordPos_size = b;
    for (a = 0; a < vocab_hash_size; a++) vocabWordPos_hash[a] = -1;
    for (a = 0; a < vocabWordPos_size; a++) {
        // Hash will be re-computed, as it is not actual
        hash = GetWordHash(vocabWordPos[a].cell);
        while (vocabWordPos_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
        vocabWordPos_hash[hash] = a;
    }
    min_reduce++;
}

void Vocab::ReduceVocabWordPosRel(){
    int a,hash;
    unsigned int b = 0;
    for (a = 0; a < vocabWordPosRel_size; a++)
        if (vocabWordPosRel[a].cn > min_reduce) {
            vocabWordPosRel[b].cn = vocabWordPosRel[a].cn;
            vocabWordPosRel[b].cell = vocabWordPosRel[a].cell;
            b++;
        } else
            free(vocabWordPosRel[a].cell);
    vocabWordPosRel_size = b;
    for (a = 0; a < vocab_hash_size; a++) vocabWordPosRel_hash[a] = -1;
    for (a = 0; a < vocabWordPosRel_size; a++) {
        // Hash will be re-computed, as it is not actual
        hash = GetWordHash(vocabWordPosRel[a].cell);
        while (vocabWordPosRel_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
        vocabWordPosRel_hash[hash] = a;
    }
    min_reduce++;
}

void Vocab::ReduceVocabRelWordPos(){
    int a,hash;
    unsigned int b = 0;
    for (a = 0; a < vocabRelWordPos_size; a++)
        if (vocabRelWordPos[a].cn > min_reduce) {
            vocabRelWordPos[b].cn = vocabRelWordPos[a].cn;
            vocabRelWordPos[b].cell = vocabRelWordPos[a].cell;
            b++;
        } else
            free(vocabRelWordPos[a].cell);
    vocabRelWordPos_size = b;
    for (a = 0; a < vocab_hash_size; a++) vocabRelWordPos_hash[a] = -1;
    for (a = 0; a < vocabRelWordPos_size; a++) {
        // Hash will be re-computed, as it is not actual
        hash = GetWordHash(vocabRelWordPos[a].cell);
        while (vocabRelWordPos_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
        vocabRelWordPos_hash[hash] = a;
    }
    min_reduce++;
}

void Vocab::SaveVocab(const char *save_vocab_file) {
    long long i;
    FILE *fo = fopen(save_vocab_file, "wb");
    fprintf(fo, "train_trees %lld\n",train_trees);
    fprintf(fo, "total_words %lld\n",total_words);
    for (i = 0; i < vocab_size; i++) fprintf(fo, "%s %lld\n", vocab[i].word, vocab[i].cn);
    fclose(fo);
}


void Vocab::ReadVocab(const char *read_vocab_file) {
    long long a;
    char line[MAX_STRING];
    FILE *fin = fopen(read_vocab_file, "rb");
    if (fin == NULL) {
        printf("Vocabulary file not found\n");
        exit(1);
    }
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    vocab_size = 0;

    int i=0;
    char ch='\n';
    while(ch!=' '){
        ch = fgetc(fin);
        line[i]=ch;
        i++;
        assert(i<MAX_STRING);
    }
    line[i-1]=0;
    assert(!strcmp(line,"train_trees"));
    fscanf(fin, "%lld%c",&train_trees, &ch);
    assert(ch=='\n');

    i=0;
    while(ch!=' '){
        ch = fgetc(fin);
        line[i]=ch;
        i++;
        assert(i<MAX_STRING);
    }
    line[i-1]=0;
    assert(!strcmp(line,"total_words"));
    fscanf(fin, "%lld%c", &total_words, &ch);
    assert(ch=='\n');

    while (true) {
        i=0;
        while(ch!=' ' && !feof(fin)){
            ch = fgetc(fin);
            line[i]=ch;
            i++;
        }
        if(feof(fin))
            break;
        assert(i<MAX_STRING);
        line[i-1]=0;
        a=AddWordToVocab(line);
        fscanf(fin, "%lld%c", &vocab[a].cn, &ch);
        assert(ch=='\n');
    }
    SortVocab();
    printf("Vocab size: %lld\n", vocab_size);
    printf("Trees in train file: %lld\n", train_trees);
    printf("Total Words in train file: %lld\n",total_words);
    fclose(fin);

}

long long Vocab::GetTotalWords() const {
    return total_words;
}

long long Vocab::GetTrainTrees() const {
    return train_trees;
}

long long Vocab::GetTrainWords() const {
    return train_words;
}

int Vocab::ReadWordFromTrainFile(char *word1,char *word2,FILE *fin) {
    char temp[MAX_STRING];
    char rel[MAX_STRING];
    while (true) {
        if (feof(fin))
            return 0;
        fgets(temp, MAX_STRING, fin);
        if (!strcmp(temp, "\n"))// find a new tree
            train_trees++;
        int i = 0;
        for (i = 0; i < strlen(temp) - 1; i++) {
            if (!isdigit(temp[i]))
                break;
        }

        bool flag= false;
        if (i < strlen(temp) - 1) {// a line of words
            assert(strlen(temp) > 4);// can be slip 4 string
            const char *d = " ";
            char *p;
            char *q = temp;

            p = strsep(&q, d);// dep_relationship
            assert(strlen(p) > 0);
            strcpy(rel,p);

            p = strsep(&q, d);// parent
            assert(strlen(p) > 0);

            for (int k = 0; k < strlen(p); k++) {
                if (isalpha(p[k])) {// not punct
                    strcpy(word1, p);
                }
            }

            p = strsep(&q, d);
            assert(strlen(p) > 0);//parent position in sentence

            if (atoi(p) == 0 && !strcmp(word1, "ROOT"))// not root
                word1[0] = 0;

            p = strsep(&q, d);
            assert(strlen(p)>0);//child
            for (int k = 0; k < strlen(p); k++) {
                if (isalpha(p[k])) {//not punct
                    strcpy(word2, p);
                    flag= true;
                }
            }

        }
        return 0;
    }
}


void Vocab::LearnVocabFromTrainFile(const char *train_file) {
    char word1[MAX_STRING],word2[MAX_STRING];
    FILE *fin;
    long long a, i;
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    fin = fopen(train_file, "rb");
    if (fin == NULL) {
        printf("ERROR: training data file not found!\n");
        exit(1);
    }
    vocab_size = 0;
    while (1) {
        word1[0]=0;
        word2[0]=0;
        ReadWordFromTrainFile(word1,word2, fin);
        if(strlen(word1)==0 && strlen(word2)==0 && feof(fin))
            break;
        assert(strlen(word1)>0 || strlen(word2)>0);

        if( tree_degree && strlen(word1)>0) {
            i = SearchVocab(word1);
            if (i == -1)
                AddWordToVocab(word1);
            else vocab[i].cn++;
            if (vocab_size > vocab_hash_size * 0.7)
                ReduceVocab();
        }

        if(strlen(word2)>0) {
            i = SearchVocab(word2);
            if (i == -1)
                AddWordToVocab(word2);
            else vocab[i].cn++;
            train_words++;
            total_words++;
            if (vocab_size > vocab_hash_size * 0.7)
                ReduceVocab();
        }

        if (feof(fin)) break;
    }

    SortVocab();
    printf("Vocab size: %lld\n", vocab_size);
    printf("Trees in train file: %lld\n", train_trees);
    printf("Total Words in train file: %lld\n",total_words);

    fclose(fin);
}

Vocab::~Vocab() {
    int a;

    for (a = 0; a < vocab_size; a++) {
        if (vocab[a].word != NULL) {
            free(vocab[a].word);
        }

    }
    free(vocab[vocab_size].word);
    free(vocab);
}
    ClearVocab();
    free(vocab_hash);
}
