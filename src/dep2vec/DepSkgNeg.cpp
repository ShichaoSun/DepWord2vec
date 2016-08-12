//
// Created by bruce on 7/13/16.
//

#include "DepSkgNeg.h"

DepSkgNeg::DepSkgNeg(const Vocab& v):table_size(1e8),vocab(v){//initialize default parameter
    alpha=0.025;
    starting_alpha=0.025;
    binary=0;
    iter=5;
    file_size=0;
    bigdata=0;
    debug_mode=2;
    layer1_size=200;
    word_count_total=0;
    word_count_actual=0;
    tree_count_actual=0;
    train_file[0]=0;
    fromTempfile[0]=0;
    syn0=NULL;
    syn1neg=NULL;
    expTable = (real *)malloc((EXP_TABLE_SIZE + 1) * sizeof(real));
    if (expTable == NULL) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    for (int i = 0; i < EXP_TABLE_SIZE; i++) {
        expTable[i] = exp((i / (real)EXP_TABLE_SIZE * 2 - 1) * MAX_EXP); // Precompute the exp() table
        expTable[i] = expTable[i] / (expTable[i] + 1);                   // Precompute f(x) = x / (x + 1)
    }
    table=NULL;
    sample=1e-4;
    negative=5;
    num_threads=8;
    window=5;

    //InitNet
    long long a, b;
    unsigned long long next_random = 1;
    long long vocab_size=vocab.GetVocabSize();
    posix_memalign((void **)&syn0, 128,  vocab_size * layer1_size * sizeof(real));

    if (syn0 == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    posix_memalign((void **)&syn1neg, 128,  (long long)vocab_size * layer1_size * sizeof(real));

    if (syn1neg == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    for (a = 0; a < vocab_size; a++) for (b = 0; b < layer1_size; b++)
            syn1neg[a * layer1_size + b] = 0;


    for (a = 0; a < vocab_size; a++) for (b = 0; b < layer1_size; b++) {
            next_random = next_random * (unsigned long long)25214903917 + 11;
            syn0[a * layer1_size + b] = (((next_random & 0xFFFF) / (real)65536) - 0.5) / layer1_size;
        }

}

//Set parameter from user configuration
void DepSkgNeg::Setlayer1_size(long long x){
    layer1_size=x;
}

void DepSkgNeg::SetBigdata(int x){
    bigdata=x;
}

void DepSkgNeg::SetTrainfile(const char *f) {
    strcpy(train_file,f);
}

void DepSkgNeg::SetFromTempfile(const char *f) {
    strcpy(fromTempfile,f);
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
void DepSkgNeg::SetWindow(int x){
    window=x;
}

void DepSkgNeg::SetNegative(int x){
    negative=x;
}

void DepSkgNeg::SetIter(int x) {
    iter=x;
}

void DepSkgNeg::InitUnigramTable() {// initialize a table for negative sampling
    int a, i;
    double train_words_pow = 0;
    double d1, power = 0.75;
    table = (int *)malloc(table_size * sizeof(int));
    for (a = 0; a < vocab.GetVocabSize(); a++) train_words_pow += pow(vocab.GetVocabWordCn(a), power);
    i = 0;
    d1 = pow(vocab.GetVocabWordCn(i), power) / train_words_pow;
    for (a = 0; a < table_size; a++) {
        table[a] = i;
        if (a / (double)table_size > d1) {
            i++;
            d1 += pow(vocab.GetVocabWordCn(i), power) / train_words_pow;
        }
        if (i >= vocab.GetVocabSize()) i = vocab.GetVocabSize() - 1;
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

void DepSkgNeg::TrainModelThread(long long id){
    long long a, b, d, word, last_word;
    int sentence_position = 0, sentence_length = 0;
    long long total_word_count = 0, last_total_word_count = 0,train_word_count=0,last_train_word_count=0;
    long long tree_count=0,last_tree_count=0;
    long long l1, l2, c, target, label;
    int local_iter=iter;
    unsigned long long next_random = id;
    real f, g;
    clock_t now,last_clock=start;

    real *neu1e = (real *)calloc(layer1_size, sizeof(real));
    FILE *fi = fopen(train_file, "rb");
    if (fi == NULL) {
        fprintf(stderr, "no such file or directory: %s", train_file);
        exit(1);
    }

    fseek(fi, file_size / (long long)num_threads * id, SEEK_SET);
    FindTreeStart(fi);


    DepTree depTree(vocab);

    long long train_words=vocab.GetTrainWords();
    long long train_trees=vocab.GetTrainTrees();

    while (1) {
        if (tree_count - last_tree_count >= 100) {  //update the learning rate for every 100 trees
            tree_count_actual += tree_count - last_tree_count;  //global count of trees
            word_count_actual += train_word_count - last_train_word_count;
            word_count_total  += total_word_count - last_total_word_count;
            last_tree_count = tree_count;
            last_total_word_count = total_word_count;
            last_train_word_count = train_word_count;
            if ((debug_mode > 1)) {
                now = clock();
                printf("%cAlpha: %f  Tree Progress: %.2f%%  Trees/thread/sec: %.2fk  Words/thread/sec: %.2fk ", 13,
                       alpha,
                       tree_count_actual / (real) (iter * train_trees + 1) * 100,
                       tree_count_actual / ((real) (now - start + 1) / (real) CLOCKS_PER_SEC * 1000),
                       word_count_actual / ((real) (now - start + 1) / (real) CLOCKS_PER_SEC * 1000));
                fflush(stdout);
            }
            alpha = starting_alpha * (1 - tree_count_actual / (real) (iter * train_trees + 1));
            if (alpha < starting_alpha * 0.0001) alpha = starting_alpha * 0.0001;

            if(id==0 && bigdata==1){
                now=clock();
                if((real) ( now - last_clock + 1) / (real) CLOCKS_PER_SEC > 50){
                    last_clock=now;
                    FILE *ftemp = fopen("temp", "wb");
                    fprintf(ftemp, "%d\n", local_iter);
                    fprintf(ftemp, "%lf\n", alpha);
                    for (long long ta = 0; ta < vocab.GetVocabSize(); ta++)
                        for (long long tb = 0; tb < layer1_size; tb++)
                            fprintf(ftemp, "%lf ", syn0[ta * layer1_size + tb]);
                    fprintf(ftemp, "\n");
                    for (long long ta = 0; ta < vocab.GetVocabSize(); ta++)
                        for (long long tb = 0; tb < layer1_size; tb++)
                            fprintf(ftemp, "%lf ", syn1neg[ta * layer1_size + tb]);
                    fprintf(ftemp, "\n");
                }
            }

        }

        //Step1: load a tree into mem
        depTree.GetDepTreeFromFilePointer(fi);
        sentence_length = depTree.GetSenlen();
        if (sentence_length != -1) {
            tree_count++;
            total_word_count += sentence_length;
            train_word_count += depTree.GetWordCountActual();
        } else if (feof(fi)) {
            tree_count_actual += tree_count - last_tree_count;
            word_count_actual += train_word_count - last_train_word_count;
            word_count_total += total_word_count - last_total_word_count;
            local_iter--;
            if (local_iter == 0)
                break;
            tree_count = 0;
            last_tree_count = 0;
            train_word_count = 0;
            total_word_count =0;
            last_train_word_count = 0;
            last_total_word_count = 0;
            fseek(fi, file_size / (long long) num_threads * id, SEEK_SET);
            FindTreeStart(fi);
            continue;
        } else
            continue;

        //step2:train a tree
        for (sentence_position = 1; sentence_position <= sentence_length; sentence_position++) {

            word = depTree.GetWordInPos(sentence_position);

            if (word != -1) {


                // The subsampling randomly discards frequent words while keeping the ranking same
                if (sample > 0) {
                    real ran = (sqrt(vocab.GetVocabWordCn(word) / (sample * train_words)) + 1) * (sample * train_words) / vocab.GetVocabWordCn(word);
                    next_random = next_random * (unsigned long long)25214903917 + 11;
                    if (ran < (next_random & 0xFFFF) / (real)65536)
                        continue;

                }

                for (c = 0; c < layer1_size; c++) neu1e[c] = 0;  //vector to accumulate the updates
                next_random = next_random * (unsigned long long) 25214903917 + 11;
                b = next_random % window + 1;
                vector<int> sam = depTree.GetSample(sentence_position,b,sample,next_random);

                unsigned long s_size = sam.size();
                if(s_size==0) continue;
                for (a = 0; a < s_size; a++) {
                    c = sam[a];
                    last_word = depTree.GetWordInPos(c);
                    assert(last_word != -1);

                    l1 = last_word * layer1_size;
                    for (c = 0; c < layer1_size; c++) neu1e[c] = 0;

                    // NEGATIVE SAMPLING
                    if (negative > 0)
                        for (d = 0; d < negative + 1; d++) {
                            if (d == 0) {
                                target = word;
                                label = 1;
                            } else {
                                next_random = next_random * (unsigned long long) 25214903917 + 11;
                                target = table[(next_random >> 16) % table_size];
                                long long vocab_size = vocab.GetVocabSize();
                                if (target == 0) target = next_random % (vocab_size - 1) + 1;
                                if (target == word) continue;
                                label = 0;
                            }
                            l2 = target * layer1_size;
                            f = 0;
                            for (c = 0; c < layer1_size; c++) f += syn0[c + l1] * syn1neg[c + l2];
                            if (f > MAX_EXP) g = (label - 1) * alpha;
                            else if (f < -MAX_EXP) g = (label - 0) * alpha;
                            else g = (label - expTable[(int) ((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                            for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
                            for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * syn0[c + l1];
                        }
                    // Learn weights input -> hidden
                    for (c = 0; c < layer1_size; c++) syn0[c + l1] += neu1e[c];
                }
            }
        }

        if(tree_count > train_trees/num_threads){// every thread can train trees ,not more than train_trees/num_threads
            tree_count_actual += tree_count - last_tree_count;
            word_count_actual += train_word_count - last_train_word_count;
            word_count_total += total_word_count - last_total_word_count;
            local_iter--;// next iteration
            if (local_iter == 0)
                break;
            tree_count = 0;
            last_tree_count = 0;
            train_word_count = 0;
            total_word_count =0;
            last_train_word_count = 0;
            last_total_word_count = 0;
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
    long a;
    pthread_t *pt = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    if (pt == NULL) {
        fprintf(stderr, "cannot allocate memory for threads\n");
        exit(1);
    }

    if(bigdata==1 && strlen(fromTempfile)>0){
        printf("Continuing training using file %s\n", fromTempfile);
        FILE *fitemp=fopen(fromTempfile,"rb");
        if (fitemp == NULL) {
            printf("Temp file not found\n");
            exit(1);
        }
        fscanf(fitemp,"%d\n",&iter);
        fscanf(fitemp, "%lf\n",&alpha);

        for (long long ta = 0; ta < vocab.GetVocabSize(); ta++)
            for (long long tb = 0; tb < layer1_size; tb++)
                fscanf(fitemp, "%lf ", &syn0[ta * layer1_size + tb]);
        fscanf(fitemp, "\n");
        for (long long ta = 0; ta < vocab.GetVocabSize(); ta++)
            for (long long tb = 0; tb < layer1_size; tb++)
                fscanf(fitemp, "%lf ", &syn1neg[ta * layer1_size + tb]);
    } else
        printf("Starting training using file %s\n", train_file);

    start = clock();
    InitUnigramTable();

    Para param[num_threads];
    for (a = 0; a < num_threads; a++) {
        param[a].pSelf=this;
        param[a].id=a;
    }

    for (a = 0; a < num_threads; a++) pthread_create(&pt[a], NULL, BasicTrainModelThread,&param[a]);
    for (a = 0; a < num_threads; a++) pthread_join(pt[a], NULL);

    printf("\nTrees trained: %lld\n", tree_count_actual/iter);
    printf("Words trained: %lld\n", word_count_actual/iter);
    printf("Total Words visited: %lld\n", word_count_total/iter);

    free(pt);

}

void DepSkgNeg::SaveWordVectors(const char *output_file) {
    FILE *fo = fopen(output_file, "wb");
    if (fo == NULL) {
        fprintf(stderr, "Cannot open %s: permission denied\n", output_file);
        exit(1);
    }
    long long a=0,b=0;
    // Save the word vectors
    long long vocab_size=vocab.GetVocabSize();
    fprintf(fo, "%lld %lld\n", vocab_size, layer1_size);
    for (a = 0; a < vocab_size; a++) {
        const char* wordOfa=vocab.GetVocabWord(a);
        if ( wordOfa!= NULL) {
            fprintf(fo,"%s ",wordOfa);
        }
        if (binary) for (b = 0; b < layer1_size; b++) fwrite(&syn0[a * layer1_size + b], sizeof(real), 1, fo);
        else for (b = 0; b < layer1_size; b++) fprintf(fo, "%lf ", syn0[a * layer1_size + b]);
        fprintf(fo, "\n");
    }
}

