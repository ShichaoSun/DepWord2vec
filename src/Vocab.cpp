//
// Created by bruce on 16-7-4.
//

#include "Vocab.h"
Vocab::Vocab():vocab_hash_size(30000000){
    vocab_max_size=1000;
    min_count=5;
    min_reduce=1;
    vocab_size=0;
    vocab=(Word*)calloc(vocab_max_size,sizeof(Word));
    vocab_hash=(int*)calloc(vocab_hash_size,sizeof(int));
}

int Vocab::GetWordHash(const char *word) {// Returns hash value of a word
    unsigned long long a, hash = 0;
    for (a = 0; a < strlen(word); a++) hash = hash * 257 + word[a];
    hash = hash % vocab_hash_size;
    return hash;
}

int Vocab::SearchVocab(const char *word) {// Returns position of a word in the vocabulary; if the word is not found, returns -1
    unsigned int hash = GetWordHash(word);
    while (1) {
        if (vocab_hash[hash] == -1) return -1;
        //if (!strcmp(word, vocab[vocab_hash[hash]].word)) return vocab_hash[hash];
        if (!strcmp(word, vocab[vocab_hash[hash]].GetWord())) return vocab_hash[hash];
        hash = (hash + 1) % vocab_hash_size;
    }
    return -1;
}

int Vocab::AddWordToVocab(Word *w) {
    unsigned int hash, length = strlen(w->GetWord()) + 1;
    if (length > MAX_STRING) length = MAX_STRING;
    vocab[vocab_size].CreatWordSpace(length);
    vocab[vocab_size].SetWord(w->GetWord());
    vocab[vocab_size].SetCN(w->GetCN());
    vocab_size++;
    // Reallocate memory if needed
    if (vocab_size + 2 >= vocab_max_size) {
        vocab_max_size += 1000;
        vocab = (Word*)realloc(vocab, vocab_max_size * sizeof(Word));
    }
    hash = GetWordHash(w->GetWord());
    while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
    vocab_hash[hash] = vocab_size - 1;
    return vocab_size - 1;
}

int Vocab::AddWordToVocab(char *word) {// Adds a word to the vocabulary
    unsigned int hash, length = strlen(word) + 1;
    if (length > MAX_STRING) length = MAX_STRING;
    //vocab[vocab_size].word = (char *)calloc(length, sizeof(char));
    //strcpy(vocab[vocab_size].word, word);
    //vocab[vocab_size].cn = 0;
    vocab[vocab_size].CreatWordSpace(length);
    vocab[vocab_size].SetWord(word);
    vocab[vocab_size].SetCN(0);
    vocab_size++;
    // Reallocate memory if needed
    if (vocab_size + 2 >= vocab_max_size) {
        vocab_max_size += 1000;
        vocab = (Word*)realloc(vocab, vocab_max_size * sizeof(Word));
    }
    hash = GetWordHash(word);
    while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
    vocab_hash[hash] = vocab_size - 1;
    return vocab_size - 1;
}

static int VocabCompare(const void*a,const void *b){
    // return ((struct vocab_word *)b)->cn - ((struct vocab_word *)a)->cn;
    return (((Word*)b)->GetCN() - ((Word *)a)->GetCN());
}

long long Vocab::GetVocabSize(){
    return vocab_size;
}

long long Vocab::GetVocabWordCn(long long i){
    return vocab[i].GetCN();
}

void Vocab::DestroyVocab() {
    int a;

    for (a = 0; a < vocab_size; a++) {
        if (vocab[a].GetWord() != NULL) {
            vocab[a].~Word();
        }
        /*
        if (vocab[a].code != NULL) {
            free(vocab[a].code);
        }
        if (vocab[a].point != NULL) {
            free(vocab[a].point);
        }
         */
    }
    //free(vocab[vocab_size].word);
    vocab[vocab_size].~Word();
    free(vocab);
}

void Vocab::SortVocab(){
    int a, size;
    unsigned int hash;
    // Sort the vocabulary and keep </s> at the first position
    qsort(&vocab[1], vocab_size - 1, sizeof(Word), VocabCompare);
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    size = vocab_size;
    train_words = 0;
    for (a = 1; a < size; a++) { // Skip </s>
        // Words occuring less than min_count times will be discarded from the vocab
        // if (vocab[a].cn < min_count) {
        if (vocab[a].GetCN() < min_count) {
            vocab_size--;
            // free(vocab[a].word);
            vocab[a].~Word();
        } else {
            // Hash will be re-computed, as after the sorting it is not actual
            hash=GetWordHash(vocab[a].GetWord());
            while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
            vocab_hash[hash] = a;
            //train_words +=vocab[a].cn;
            train_words += vocab[a].GetCN();
        }
    }
    vocab = (Word *)realloc(vocab, (vocab_size + 1) * sizeof(Word));
    // Allocate memory for the binary tree construction
    /*
    for (a = 0; a < vocab_size; a++) {
        vocab[a].code = (char *)calloc(MAX_CODE_LENGTH, sizeof(char));
        vocab[a].point = (int *)calloc(MAX_CODE_LENGTH, sizeof(int));
    }
     */
}

void Vocab::ReduceVocab() {
    int a, b = 0;
    unsigned int hash;
    //for (a = 0; a < vocab_size; a++) if (vocab[a].cn > min_reduce) {
    for (a = 0; a < vocab_size; a++) if (vocab[a].GetCN() > min_reduce) {
            //vocab[b].cn = vocab[a].cn;
            //vocab[b].word = vocab[a].word;
            vocab[b].SetCN(vocab[a].GetCN());
            vocab[b].SetWord(vocab[a].GetWord());
            b++;
        } else
            //free(vocab[a].word);
            vocab[a].~Word();
    vocab_size = b;
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    for (a = 0; a < vocab_size; a++) {
        // Hash will be re-computed, as it is not actual
        hash = GetWordHash(vocab[a].GetWord());
        while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
        vocab_hash[hash] = a;
    }
    //fflush(stdout);
    min_reduce++;
}

void Vocab::SaveVocab(const char *save_vocab_file) {
    long long i;
    FILE *fo = fopen(save_vocab_file, "wb");
    //for (i = 0; i < vocab_size; i++) fprintf(fo, "%s %lld\n", vocab[i].word, vocab[i].cn);
    for (i = 0; i < vocab_size; i++) fprintf(fo, "%s %lld\n", vocab[i].GetWord(), vocab[i].GetCN());
    fclose(fo);
}

void Vocab::ReadVocab(const char *read_vocab_file) {
    long long a;// i = 0;
    //char c;
    //char word[MAX_STRING];
    FILE *fin = fopen(read_vocab_file, "rb");
    if (fin == NULL) {
        printf("Vocabulary file not found\n");
        exit(1);
    }
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    vocab_size = 0;
    while (1) {
        //ReadWord(word, fin);
        Word w(fin);
        Word *pw=&w;
        if (feof(fin)) break;
        //a=AddWordToVocab(word);
        AddWordToVocab(pw);
        w.~Word();
        //fscanf(fin, "%lld%c", &vocab[a].cn, &c);
        //i++;
    }
    SortVocab();
    fclose(fin);
    /*
    if (debug_mode > 0) {
        printf("Vocab size: %lld\n", vocab_size);
        printf("Words in train file: %lld\n", train_words);
    }

    fin = fopen(train_file, "rb");
    if (fin == NULL) {
        printf("ERROR: training data file not found!\n");
        exit(1);
    }
    fseek(fin, 0, SEEK_END);
    file_size = ftell(fin);
    fclose(fin);
     */
}

long long Vocab::GetTrainWords(){
    return train_words;
}

Word* Vocab::GetVocab() {
    return vocab;
}

void Vocab::LearnVocabFromTrainFile(const char *train_file) {
    //char word[MAX_STRING];
    FILE *fin;
    long long a, i;
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    fin = fopen(train_file, "rb");
    if (fin == NULL) {
        printf("ERROR: training data file not found!\n");
        exit(1);
    }
    vocab_size = 0;
    AddWordToVocab((char *)"</s>");
    while (1) {
        Word w;
        w.ReadWord(fin);
        //ReadWord(word, fin);
        if (feof(fin)) break;
        train_words++;
        /*
        if ((debug_mode > 1) && (train_words % 100000 == 0)) {
            printf("%lldK%c", train_words / 1000, 13);
            fflush(stdout);
        }
         */
        //i = SearchVocab(word);
        i=SearchVocab(w.GetWord());
        if (i == -1) {
            w.SetCN(1);
            AddWordToVocab(&w);
            //a = AddWordToVocab(word);
            //vocab[a].cn = 1;
        } else vocab[i].CNincrement();
        if (vocab_size > vocab_hash_size * 0.7) ReduceVocab();
    }
    SortVocab();
    /*
    if (debug_mode > 0) {
        printf("Vocab size: %lld\n", vocab_size);
        printf("Words in train file: %lld\n", train_words);
    }
    file_size = ftell(fin);
*/
    fclose(fin);
}

Vocab::~Vocab() {
    free(vocab);
    free(vocab_hash);
}