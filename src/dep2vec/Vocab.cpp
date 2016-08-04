//
// Created by bruce on 7/13/16.
//

#include "Vocab.h"

Vocab::Vocab():vocab_hash_size(30000000){
    train_words=0;
    train_trees=-2;//two empty line at the begin of file
    total_words=0;
    vocab_max_size=1000;
    min_count=5;
    min_reduce=1;
    vocab_size=0;
    vocab=(vocab_word*)calloc(vocab_max_size,sizeof(vocab_word));
    vocab_hash=(int*)calloc(vocab_hash_size,sizeof(int));
}

int Vocab::GetWordHash(const char *word) const{// Returns hash value of a word
    unsigned long long a, hash = 0;
    for (a = 0; a < strlen(word); a++) hash = hash * 257 + word[a];
    hash = hash % vocab_hash_size;
    return hash;
}

int Vocab::SearchVocab(const char *word) const{// Returns position of a word in the vocabulary; if the word is not found, returns -1
    unsigned int hash = GetWordHash(word);
    while (1) {
        if (vocab_hash[hash] == -1) return -1;
        if (!strcmp(word,vocab[vocab_hash[hash]].word)) return vocab_hash[hash];
        hash = (hash + 1) % vocab_hash_size;
    }
}

char * Vocab::GetVocabWord(long long a) const {
    return vocab[a].word;
}

int Vocab::AddWordToVocab(char *word) {// Adds a word to the vocabulary
    unsigned int hash, length = strlen(word) + 1;
    if (length > MAX_STRING) length = MAX_STRING;

    vocab[vocab_size].word = (char *)calloc(length, sizeof(char));
    strcpy(vocab[vocab_size].word, word);
    vocab[vocab_size].cn = 1;
    vocab_size++;
    // Reallocate memory if needed
    if (vocab_size + 2 >= vocab_max_size) {
        vocab_max_size += 1000;
        vocab = (vocab_word*)realloc(vocab, vocab_max_size * sizeof(vocab_word));
    }
    hash = GetWordHash(word);
    while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
    vocab_hash[hash] = vocab_size - 1;
    return vocab_size - 1;
}

static int VocabCompare(const void*a,const void *b){
    return (((vocab_word*)b)->cn - ((vocab_word*)a)->cn);
}

long long Vocab::GetVocabSize() const{
    return vocab_size;
}

long long Vocab::GetVocabWordCn(long long i) const {
    return vocab[i].cn;
}

void Vocab::SetMincount(int x){
    min_count=x;
}

void Vocab::ClearVocab(){
    int a;

    for (a = 0; a < vocab_size; a++) {
        if (vocab[a].word != NULL) {
            free(vocab[a].word);
        }

    }
    free(vocab[vocab_size].word);
    free(vocab);
}

void Vocab::SortVocab(){
    int a, size;
    unsigned int hash;
    // Sort the vocabulary and keep </s> at the first position
    qsort(&vocab[0], vocab_size, sizeof(vocab_word), VocabCompare);
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    size = vocab_size;
    train_words = 0;
    for (a = 0; a < size; a++) {
        // Words occuring less than min_count times will be discarded from the vocab
        if (vocab[a].cn < min_count) {
            vocab_size--;
            free(vocab[a].word);
            vocab[a].word = NULL;
        } else {
            // Hash will be re-computed, as after the sorting it is not actual
            hash=GetWordHash(vocab[a].word);
            while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
            vocab_hash[hash] = a;
            train_words +=vocab[a].cn;
        }
    }
    vocab = (vocab_word*)realloc(vocab, (vocab_size + 1) * sizeof(vocab_word));
    // Allocate memory for the binary tree construction

}

void Vocab::ReduceVocab() {
    int a, b = 0;
    unsigned int hash;
    for (a = 0; a < vocab_size; a++)
        if (vocab[a].cn > min_reduce) {
            vocab[b].cn = vocab[a].cn;
            vocab[b].word = vocab[a].word;
            b++;
        } else
            free(vocab[a].word);
    vocab_size = b;
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    for (a = 0; a < vocab_size; a++) {
        // Hash will be re-computed, as it is not actual
        hash = GetWordHash(vocab[a].word);
        while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
        vocab_hash[hash] = a;
    }
    min_reduce++;
}

void Vocab::SaveVocab(const char *save_vocab_file) {
    long long i;
    FILE *fo = fopen(save_vocab_file, "wb");
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
    while (!feof(fin)) {
        int i=0;
        char ch='\n';
        while(ch!=' '){
            ch = fgetc(fin);
            line[i]=ch;
            i++;
            assert(i<MAX_STRING);
        }
        line[i]=0;
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

        if (i < strlen(temp) - 1) {// a line of words
            assert(strlen(temp) > 4);// can be slip 4 string
            const char *d = " ";
            char *p;
            char *q = temp;

            p = strsep(&q, d);// dep_relationship
            assert(strlen(p) > 0);

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
                    return 0;
                }
            }
        }

        if (i == strlen(temp) - 1)// assert is an end of tree
            assert(temp[i] == '\n');

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
        total_words++;

        if(strlen(word1)>0) {
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
    ClearVocab();
    free(vocab_hash);
}
