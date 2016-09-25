//
// Created by bruce on 7/13/16.
//

#include "Vocab.h"

Vocab::Vocab():vocab_hash_size(300000000){//initialize the default parameter
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

    for (unsigned int a = 0; a < vocab_hash_size; a++) vocabWord_hash[a] = -1;
    for (unsigned int a = 0; a < vocab_hash_size; a++) vocabWordPos_hash[a] = -1;
    for (unsigned int a = 0; a < vocab_hash_size; a++) vocabWordPosRel_hash[a] = -1;
    for (unsigned int a = 0; a < vocab_hash_size; a++) vocabRelWordPos_hash[a] = -1;
}

int Vocab::GetWordHash(const char *word) const{// Returns hash value of a word
    int a, hash = 0;
    for (a = 0; a < strlen(word); a++) hash = hash * 257 + word[a];
    hash = hash % vocab_hash_size;
    return hash;
}

int Vocab::GetWordPosHash(const char *wordPos) const {// Returns hash value of a wordpos
    int a, hash = 0;
    for (a = 0; a < strlen(wordPos); a++) hash = hash * 257 + wordPos[a];
    hash = hash % vocab_hash_size;
    return hash;
}

int Vocab::GetWordPosRelHash(const char *wordPosRel) const {// Returns hash value of a wordposrel
    int a, hash = 0;
    for (a = 0; a < strlen(wordPosRel); a++) hash = hash * 257 + wordPosRel[a];
    hash = hash % vocab_hash_size;
    return hash;
}

int Vocab::GetRelWordPosHash(const char *relWordPosRel) const {// Returns hash value of a relwordpos
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
    while (vocabWordPos_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
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
    while (vocabWordPosRel_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
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
    while (vocabRelWordPos_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
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
    unsigned int i;
    FILE *fo = fopen(save_vocab_file, "wb");
    fprintf(fo, "train_trees %lld\n",train_trees);
    fprintf(fo, "total_words %lld\n",total_words);
    fprintf(fo,"Word %u\n",vocabWord_size);
    for (i = 0; i < vocabWord_size; i++) fprintf(fo, "%s %lld\n", vocabWord[i].cell, vocabWord[i].cn);
    fprintf(fo,"WordPos %u\n",vocabWordPos_size);
    for (i = 0; i < vocabWordPos_size; i++) fprintf(fo, "%s %lld\n", vocabWordPos[i].cell, vocabWordPos[i].cn);
    fprintf(fo,"WordPosRel %u\n",vocabWordPosRel_size);
    for (i = 0; i < vocabWordPosRel_size; i++) fprintf(fo, "%s %lld\n", vocabWordPosRel[i].cell, vocabWordPosRel[i].cn);
    fprintf(fo,"RelWordPos %u\n",vocabRelWordPos_size);
    for (i = 0; i < vocabRelWordPos_size; i++) fprintf(fo, "%s %lld\n", vocabRelWordPos[i].cell, vocabRelWordPos[i].cn);
    fclose(fo);
    printf("Vocabs have been saved!\n");
}


void Vocab::ReadVocab(const char *read_vocab_file) {
    unsigned int t_vocabWord_size,t_vocabWordPos_size,t_vocabWordPosRel_size,t_vocabRelWordPos_size;
    char line[MAX_STRING];
    FILE *fin = fopen(read_vocab_file, "rb");
    if (fin == NULL) {
        printf("Vocabulary file not found\n");
        exit(1);
    }

    int i=0;
    char ch='\n';
    while(ch!=' '){
        ch = (char)fgetc(fin);
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
        ch = (char)fgetc(fin);
        line[i]=ch;
        i++;
        assert(i<MAX_STRING);
    }
    line[i-1]=0;
    assert(!strcmp(line,"total_words"));
    fscanf(fin, "%lld%c", &total_words, &ch);
    assert(ch=='\n');

    i=0;
    while(ch!=' '){
        ch = (char)fgetc(fin);
        line[i]=ch;
        i++;
        assert(i<MAX_STRING);
    }
    line[i-1]=0;
    assert(!strcmp(line,"Word"));
    fscanf(fin, "%u%c", &t_vocabWord_size, &ch);
    assert(ch=='\n');

    for (unsigned int k=0;k<t_vocabWord_size;k++) {
        i=0;
        while(ch!=' '){
            ch =(char)fgetc(fin);
            line[i]=ch;
            i++;
        }
        assert(i<MAX_STRING);
        line[i-1]=0;
        long long a=AddWordToVocab(line);
        fscanf(fin, "%lld%c", &vocabWord[a].cn, &ch);
        assert(ch=='\n');
    }

    SortVocabWord();

    i=0;
    while(ch!=' '){
        ch = (char)fgetc(fin);
        line[i]=ch;
        i++;
        assert(i<MAX_STRING);
    }
    line[i-1]=0;
    assert(!strcmp(line,"WordPos"));
    fscanf(fin, "%u%c", &t_vocabWordPos_size, &ch);
    assert(ch=='\n');

    for (unsigned int k=0;k<t_vocabWordPos_size;k++) {
        i=0;
        while(ch!=' '){
            ch =(char)fgetc(fin);
            line[i]=ch;
            i++;
        }
        assert(i<MAX_STRING);
        line[i-1]=0;
        long long a=AddWordPosToVocab(line);
        fscanf(fin, "%lld%c", &vocabWordPos[a].cn, &ch);
        assert(ch=='\n');
    }

    SortVocabWordPos();

    i=0;
    while(ch!=' '){
        ch = (char)fgetc(fin);
        line[i]=ch;
        i++;
        assert(i<MAX_STRING);
    }
    line[i-1]=0;
    assert(!strcmp(line,"WordPosRel"));
    fscanf(fin, "%u%c", &t_vocabWordPosRel_size, &ch);
    assert(ch=='\n');

    for (unsigned int k=0;k<t_vocabWordPosRel_size;k++) {
        i=0;
        while(ch!=' '){
            ch =(char)fgetc(fin);
            line[i]=ch;
            i++;
        }
        assert(i<MAX_STRING);
        line[i-1]=0;
        long long a=AddWordPosRelToVocab(line);
        fscanf(fin, "%lld%c", &vocabWordPosRel[a].cn, &ch);
        assert(ch=='\n');
    }

    SortVocabWordPosRel();

    i=0;
    while(ch!=' '){
        ch = (char)fgetc(fin);
        line[i]=ch;
        i++;
        assert(i<MAX_STRING);
    }
    line[i-1]=0;
    assert(!strcmp(line,"RelWordPos"));
    fscanf(fin, "%u%c", &t_vocabRelWordPos_size, &ch);
    assert(ch=='\n');

    for (unsigned int k=0;k<t_vocabRelWordPos_size;k++) {
        i=0;
        while(ch!=' '){
            ch =(char)fgetc(fin);
            line[i]=ch;
            i++;
        }
        assert(i<MAX_STRING);
        line[i-1]=0;
        long long a=AddRelWordPosToVocab(line);
        fscanf(fin, "%lld%c", &vocabRelWordPos[a].cn, &ch);
        assert(ch=='\n');
    }

    SortVocabRelWordPos();

    fclose(fin);

    printf("Vocab of word size: %u\n", vocabWord_size);
    printf("Vocab of wordPos size: %u\n", vocabWordPos_size);
    printf("Vocab of wordPosRel size: %u\n", vocabWordPosRel_size);
    printf("Vocab of relWordPos size: %u\n", vocabRelWordPos_size);
    printf("Trees in train file: %lld\n", train_trees);
    printf("Total Words in train file: %lld\n",total_words);

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

int Vocab::LearnVocabFromTrainFile(const char *train_file) {
    char word1[MAX_STRING],word2[MAX_STRING],rel[MAX_STRING],temp[MAX_STRING];
    FILE *fin;
    char *p;
    const char *d=" ";

    fin = fopen(train_file, "rb");
    if (fin == NULL) {
        printf("ERROR: training data file not found!\n");
        exit(1);
    }
    while (!feof(fin)) {

        while (!feof(fin)){
            fgets(temp,MAX_STRING,fin);  //read in a line
            if(strlen(temp)<2)
                continue;
            int j=0;
            for(j=0;j<strlen(temp)-1;j++)
                if(!isdigit(temp[j]))
                    break;
            if(j==strlen(temp)-1 && temp[j]=='\n')
                break;
            else
                continue;
        }

        if(feof(fin))
            break;

        int senlen=atoi(temp);
        assert(senlen>0);

        total_words+=senlen;
        train_trees++;

        for(int i=0;i<senlen;i++){
            word1[0]=0;
            word2[0]=0;
            rel[0]=0;

            fgets(temp,MAX_STRING,fin);

            char *q=temp;

            p = strsep(&q, d);  //dep_relationship
            assert(strlen(p)>0);
            strcpy(rel,p);

            p = strsep(&q, d);  //parent word
            assert(strlen(p)>0);
            strcpy(word1,p);

            p = strsep(&q, d);  //parent position in sentence
            assert(strlen(p)>0  && atoi(p)>-1);

            p = strsep(&q, d);  //child word
            assert(strlen(p)>0);
            strcpy(word2,p);

            p = strsep(&q, d);   //child position in sentence
            assert(strlen(p)>0 && atoi(p)>-1);

            for (int k = 0; k < strlen(word1); k++) {
                if(word1[k]=='/') break;
                if (!isalpha(word1[k])) {// is punct
                    word1[0]=0;
                    break;
                }
            }

            for (int k = 0; k < strlen(word2); k++) {
                if(word2[k]=='/') break;
                if (!isalpha(word2[k])) {// is punct
                    word2[0]=0;
                    break;
                }
            }

            if(word2[0]!=0){
                int tw=SearchVocabWordPos(word2);
                if(tw==-1)
                    AddWordPosToVocab(word2);
                else
                    vocabWordPos[tw].cn++;

                if (vocabWordPos_size > vocab_hash_size * 0.7)
                    ReduceVocabWordPos();


                char rwp[MAX_STRING];
                strcpy(rwp,rel);
                unsigned long tl=strlen(rwp);
                rwp[tl+1]=0;
                rwp[tl]='/';
                strcat(rwp,word2);
                tw=SearchVocabRelWordPos(rwp);
                if(tw==-1)
                    AddRelWordPosToVocab(rwp);
                else
                    vocabRelWordPos[tw].cn++;

                if (vocabRelWordPos_size > vocab_hash_size * 0.7)
                    ReduceVocabRelWordPos();



                for(int k=0;k < strlen(word2);k++){
                    if(word2[k]=='/'){
                        word2[k]=0;
                        break;
                    }
                }
                tw=SearchVocabWord(word2);
                if(tw==-1)
                    AddWordToVocab(word2);
                else
                    vocabWord[tw].cn++;

                if (vocabWord_size > vocab_hash_size * 0.7)
                    ReduceVocabWord();

            }

            if(word1[0]!=0 && strcmp(rel,"root")){
                unsigned long tl=strlen(word1);
                word1[tl+1]=0;
                word1[tl]='/';
                strcat(word1,rel);
                int tw=SearchVocabWordPosRel(word1);
                if(tw==-1)
                    AddWordPosRelToVocab(word1);
                else
                    vocabWordPosRel[tw].cn++;

                if (vocabWordPosRel_size > vocab_hash_size * 0.7)
                    ReduceVocabWordPosRel();

            }
        }

        fgets(temp,MAX_STRING,fin);
        assert(!strcmp(temp,"\n"));
        if(train_trees %10000==0){
            printf("%c%lldK",13,train_trees/1000);
            fflush(stdout);
        }

    }

    fclose(fin);
    printf("\nVocab of word size before sort: %u\n", vocabWord_size);
    printf("Vocab of wordPos size before sort: %u\n", vocabWordPos_size);
    printf("Vocab of wordPosRel size before sort: %u\n", vocabWordPosRel_size);
    printf("Vocab of relWordPos size before sort: %u\n", vocabRelWordPos_size);

    SortVocabWord();
    SortVocabWordPos();
    SortVocabWordPosRel();
    SortVocabRelWordPos();

    printf("\nVocab of word size: %u\n", vocabWord_size);
    printf("Vocab of wordPos size: %u\n", vocabWordPos_size);
    printf("Vocab of wordPosRel size: %u\n", vocabWordPosRel_size);
    printf("Vocab of relWordPos size: %u\n", vocabRelWordPos_size);

    printf("Trained words in train file: %lld\n", train_words);
    printf("Trees in train file: %lld\n", train_trees);
    printf("Total Words in train file: %lld\n",total_words);

    return 0;
}

Vocab::~Vocab() {
    for (int a = 0; a < vocabWord_size; a++) {
        if (vocabWord[a].cell != NULL) {
            free(vocabWord[a].cell);
        }
    }
    free(vocabWord[vocabWord_size].cell);
    free(vocabWord);
    free(vocabWord_hash);

    for (int a = 0; a < vocabWordPos_size; a++) {
        if (vocabWordPos[a].cell != NULL) {
            free(vocabWordPos[a].cell);
        }
    }
    free(vocabWordPos[vocabWordPos_size].cell);
    free(vocabWordPos);
    free(vocabWordPos_hash);

    for (int a = 0; a < vocabWordPosRel_size; a++) {
        if (vocabWordPosRel[a].cell != NULL) {
            free(vocabWordPosRel[a].cell);
        }
    }
    free(vocabWordPosRel[vocabWordPosRel_size].cell);
    free(vocabWordPosRel);
    free(vocabWordPosRel_hash);

    for (int a = 0; a < vocabRelWordPos_size; a++) {
        if (vocabRelWordPos[a].cell != NULL) {
            free(vocabRelWordPos[a].cell);
        }
    }
    free(vocabRelWordPos[vocabRelWordPos_size].cell);
    free(vocabRelWordPos);
    free(vocabRelWordPos_hash);
}
