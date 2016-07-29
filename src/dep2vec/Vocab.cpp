//
// Created by bruce on 7/13/16.
//
//
// Created by bruce on 16-7-4.
//

#include "Vocab.h"

Vocab::Vocab():vocab_hash_size(30000000){
    train_trees=-2;//two empty line at the begin of file
    train_words=0;
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
        //if (!strcmp(word, vocab[vocab_hash[hash]].word)) return vocab_hash[hash];
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
    //vocab[vocab_size].word = (char *)calloc(length, sizeof(char));
    //strcpy(vocab[vocab_size].word, word);
    //vocab[vocab_size].cn = 0;

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
    // return ((struct vocab_word *)b)->cn - ((struct vocab_word *)a)->cn;
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
    free(vocab[vocab_size].word);
    free(vocab);
}

void Vocab::SortVocab(){
    int a, size;
    unsigned int hash;
    // Sort the vocabulary and keep </s> at the first position
    qsort(&vocab[1], vocab_size - 1, sizeof(vocab_word), VocabCompare);
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    size = vocab_size;
    train_words = 0;
    for (a = 1; a < size; a++) { // Skip </s>
        // Words occuring less than min_count times will be discarded from the vocab
        // if (vocab[a].cn < min_count) {
        if (vocab[a].cn < min_count) {
            vocab_size--;
            // free(vocab[a].word);
            free(vocab[a].word);
            vocab[a].word = NULL;
        } else {
            // Hash will be re-computed, as after the sorting it is not actual
            hash=GetWordHash(vocab[a].word);
            while (vocab_hash[hash] != -1) hash = (hash + 1) % vocab_hash_size;
            vocab_hash[hash] = a;
            //train_words +=vocab[a].cn;
            train_words += vocab[a].cn;
        }
    }
    vocab = (vocab_word*)realloc(vocab, (vocab_size + 1) * sizeof(vocab_word));
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
    //fflush(stdout);
    min_reduce++;
}

void Vocab::SaveVocab(const char *save_vocab_file) {
    long long i;
    FILE *fo = fopen(save_vocab_file, "wb");
    //for (i = 0; i < vocab_size; i++) fprintf(fo, "%s %lld\n", vocab[i].word, vocab[i].cn);
    for (i = 0; i < vocab_size; i++) fprintf(fo, "%s %lld\n", vocab[i].word, vocab[i].cn);
    fclose(fo);
}

// Reads a single word from a vocab file, assuming space + tab + EOL to be word boundaries
void Vocab::ReadWordFromVocab(char *word, FILE *fin) {
    int a = 0, ch;
    while (!feof(fin)) {
        ch = fgetc(fin);
        if (ch == 13) continue;
        if ((ch == ' ') || (ch == '\t') || (ch == '\n')) {
            if (a > 0) {
                if (ch == '\n') ungetc(ch, fin);
                break;
            }
            if (ch == '\n') {
                strcpy(word, (char *)"</s>");
                return;
            } else continue;
        }
        word[a] = ch;
        a++;
        if (a >= MAX_STRING - 1) a--;   // Truncate too long words
    }
    word[a] = 0;
}

void Vocab::ReadVocab(const char *read_vocab_file) {
    long long a;// i = 0;
    char c;
    char word[MAX_STRING];
    FILE *fin = fopen(read_vocab_file, "rb");
    if (fin == NULL) {
        printf("Vocabulary file not found\n");
        exit(1);
    }
    for (a = 0; a < vocab_hash_size; a++) vocab_hash[a] = -1;
    vocab_size = 0;
    while (1) {
        ReadWordFromVocab(word, fin);
        if (feof(fin)) break;
        AddWordToVocab(word);
        fscanf(fin, "%lld%c", &vocab[a].cn, &c);
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

long long Vocab::GetTrainWords() const {
    return train_words;
}

long long Vocab::GetTrainTrees() const {
    return train_trees;
}

int Vocab::ReadWordFromTrainFile(char *word1,char *word2,FILE *fin) {
    char temp[MAX_STRING];
    while (true) {
        if (feof(fin))
            return 0;
        fgets(temp, MAX_STRING, fin);
        if (!strcmp(temp, "\n"))
            train_trees++;
        int i = 0;
        for (i = 0; i < strlen(temp) - 1; i++) {
            if (!isdigit(temp[i]))
                break;
        }

        if (i < strlen(temp) - 1) {
            assert(strlen(temp) > 4);
            const char *d = " ";
            char *p;
            char *q = temp;
            for (int j = 0; j < 4; j++) {
                p = strsep(&q, d);
                assert(strlen(p) > 0);
                if(j==1){
                    for (int k = 0; k < strlen(p); k++) {
                        if (isalpha(p[k])) {
                            strcpy(word1, p);
                            return 0;
                        }
                    }
                }
                if(j==3){
                    for (int k = 0; k < strlen(p); k++) {
                        if (isalpha(p[k])) {
                            strcpy(word2, p);
                            return 0;
                        }
                    }
                }
            }
        }

        if (i == strlen(temp) - 1)
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
    AddWordToVocab((char *)"</s>");
    while (1) {
        word1[0]=0;
        word2[0]=0;
        ReadWordFromTrainFile(word1,word2, fin);
        if(strlen(word1)==0 && strlen(word2)==0 && feof(fin))
            break;
        assert(strlen(word1)>0 || strlen(word2)>0);
        train_words++;
        /*
        if ((debug_mode > 1) && (train_words % 100000 == 0)) {
            printf("%lldK%c", train_words / 1000, 13);
            fflush(stdout);
        }
         */
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
            if (vocab_size > vocab_hash_size * 0.7)
                ReduceVocab();
        }

        if (feof(fin)) break;
    }

    SortVocab();
    printf("Vocab size: %lld\n", vocab_size);
    printf("Words in train file: %lld\n", train_words);
    printf("Trees in train file: %lld\n", train_trees);
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
    ClearVocab();
    free(vocab_hash);
}
