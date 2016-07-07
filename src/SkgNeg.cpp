//
// Created by bruce on 7/4/16.
//

#include "SkgNeg.h"

SkgNeg::SkgNeg():table_size(1e8),alpha(0.025),starting_alpha(0.025){

    //test
    strcpy(train_file,"/home/bruce/ClionProjects/DepWord2vec/bin/Debug/test.txt");
    strcpy(output_file,"/home/bruce/ClionProjects/DepWord2vec/bin/Debug/vec.txt");

    binary=0;
    file_size=0;
    debug_mode=2;
    layer1_size=200;
    word_count_actual=0;
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
    v.LearnVocabFromTrainFile(train_file);
    //InitNet
    long long a, b;
    long long vocab_size=v.GetVocabSize();
    a = posix_memalign((void **)&syn0, 128, (long long)vocab_size * layer1_size * sizeof(real));
    if (syn0 == NULL) {printf("Memory allocation failed\n"); exit(1);}
    /*
    if (hs) {
        a = posix_memalign((void **)&syn1, 128, (long long)vocab_size * layer1_size * sizeof(real));
        if (syn1 == NULL) {printf("Memory allocation failed\n"); exit(1);}
        for (b = 0; b < layer1_size; b++) for (a = 0; a < vocab_size; a++)
                syn1[a * layer1_size + b] = 0;
    }*/
    //if (negative>0) {
    //a = posix_memalign((void **)&syn1neg, 128, (long long)vocab_size * layer1_size * sizeof(real));
    posix_memalign((void **)&syn1neg, 128, (long long)vocab_size * layer1_size * sizeof(real));
    if (syn1neg == NULL) {printf("Memory allocation failed\n"); exit(1);}
    for (b = 0; b < layer1_size; b++) for (a = 0; a < vocab_size; a++)
            syn1neg[a * layer1_size + b] = 0;
    //}
    for (b = 0; b < layer1_size; b++) for (a = 0; a < vocab_size; a++)
            syn0[a * layer1_size + b] = (rand() / (real)RAND_MAX - 0.5) / layer1_size;
    //CreateBinaryTree();
}

void SkgNeg::InitUnigramTable() {
    int a, i;
    long long train_words_pow = 0;
    real d1, power = 0.75;
    table = (int *)malloc(table_size * sizeof(int));
    if (table == NULL) {
        fprintf(stderr, "cannot allocate memory for the table\n");
        exit(1);
    }
    //for (a = 0; a < vocab_size; a++) train_words_pow += pow(vocab[a].cn, power);
    for (a = 0; a < v.GetVocabSize(); a++) train_words_pow += pow(v.GetVocabWordCn(a), power);
    i = 0;
    d1 = pow(v.GetVocabWordCn(i), power) / (real)train_words_pow;
    //d1 = pow(vocab[i].cn, power) / (real)train_words_pow;
    for (a = 0; a < table_size; a++) {
        table[a] = i;
        if (a / (real)table_size > d1) {
            i++;
            d1 += pow(v.GetVocabWordCn(i), power) / (real)train_words_pow;
            //d1 += pow(vocab[i].cn, power) / (real)train_words_pow;
        }
        if (i >= v.GetVocabSize()) i = v.GetVocabSize() - 1;
        //if (i >= vocab_size) i = vocab_size - 1;
    }
}

int SkgNeg::ReadWordIndex(FILE *fin){
    char word[MAX_STRING];
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
                break;
            } else continue;
        }
        word[a] = ch;
        a++;
        if (a >= MAX_STRING - 1) a--;   // Truncate too long words
    }
    word[a] = 0;
    if (feof(fin)) return -1;
    return v.SearchVocab(word);
}

void* SkgNeg::BasicTrainModelThread(void *param){
    Para *p=(Para*)param;
    SkgNeg* pS=(SkgNeg*)((*p).th);
    pS->TrainModelThread((*p).id);
}


void SkgNeg::TrainModelThread(long long id){
    long long a, b, d, word, last_word, sentence_length = 0, sentence_position = 0;
    long long word_count = 0, last_word_count = 0, sen[MAX_SENTENCE_LENGTH + 1];
    long long l1, l2, c, target, label;
    unsigned long long next_random = id;
    real f, g;
    clock_t now;
    real *neu1 = (real *)calloc(layer1_size, sizeof(real));
    real *neu1e = (real *)calloc(layer1_size, sizeof(real));
    FILE *fi = fopen(train_file, "rb");
    if (fi == NULL) {
        fprintf(stderr, "no such file or directory: %s", train_file);
        exit(1);
    }
    fseek(fi, file_size / (long long)num_threads * (long long)id, SEEK_SET);
    int train_words=v.GetTrainWords();
    while (1) {
        if (word_count - last_word_count > 10000) {
            word_count_actual += word_count - last_word_count;
            last_word_count = word_count;
            if ((debug_mode > 1)) {
                now=clock();
                printf("%cAlpha: %f  Progress: %.2f%%  Words/thread/sec: %.2fk  ", 13, alpha,
                       word_count_actual / (real)(train_words + 1) * 100,
                       word_count_actual / ((real)(now - start + 1) / (real)CLOCKS_PER_SEC * 1000));
                fflush(stdout);
            }
            alpha = starting_alpha * (1 - word_count_actual / (real)(train_words + 1));
            if (alpha < starting_alpha * 0.0001) alpha = starting_alpha * 0.0001;
        }
        if (sentence_length == 0) {
            while (1) {
                word = ReadWordIndex(fi);
                if (feof(fi)) break;
                if (word == -1) continue;
                word_count++;
                if (word == 0) break;
                // The subsampling randomly discards frequent words while keeping the ranking same
                if (sample > 0) {
                    real ran = (sqrt(v.GetVocabWordCn(word) / (sample * train_words)) + 1) * (sample * train_words) / v.GetVocabWordCn(word);
                    //real ran = (sqrt(vocab[word].cn / (sample * train_words)) + 1) * (sample * train_words) / vocab[word].cn;
                    next_random = next_random * (unsigned long long)25214903917 + 11;
                    if (ran < (next_random & 0xFFFF) / (real)65536) continue;
                }
                sen[sentence_length] = word;
                sentence_length++;
                if (sentence_length >= MAX_SENTENCE_LENGTH) break;
            }
            sentence_position = 0;
        }
        if (feof(fi)) break;
        if (word_count > train_words / num_threads) break;
        word = sen[sentence_position];
        if (word == -1) continue;
        for (c = 0; c < layer1_size; c++) neu1[c] = 0;
        for (c = 0; c < layer1_size; c++) neu1e[c] = 0;
        next_random = next_random * (unsigned long long)25214903917 + 11;
        b = next_random % window;
        /*
        if (cbow) {  //train the cbow architecture
            // in -> hidden
            for (a = b; a < window * 2 + 1 - b; a++) if (a != window) {
                    c = sentence_position - window + a;
                    if (c < 0) continue;
                    if (c >= sentence_length) continue;
                    last_word = sen[c];
                    if (last_word == -1) continue;
                    for (c = 0; c < layer1_size; c++) neu1[c] += syn0[c + last_word * layer1_size];
                }
            if (hs) for (d = 0; d < vocab[word].codelen; d++) {
                    f = 0;
                    l2 = vocab[word].point[d] * layer1_size;
                    // Propagate hidden -> output
                    for (c = 0; c < layer1_size; c++) f += neu1[c] * syn1[c + l2];
                    if (f <= -MAX_EXP) continue;
                    else if (f >= MAX_EXP) continue;
                    else f = expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))];
                    // 'g' is the gradient multiplied by the learning rate
                    g = (1 - vocab[word].code[d] - f) * alpha;
                    // Propagate errors output -> hidden
                    for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1[c + l2];
                    // Learn weights hidden -> output
                    for (c = 0; c < layer1_size; c++) syn1[c + l2] += g * neu1[c];
                }
            // NEGATIVE SAMPLING
            if (negative > 0) for (d = 0; d < negative + 1; d++) {
                    if (d == 0) {
                        target = word;
                        label = 1;
                    } else {
                        next_random = next_random * (unsigned long long)25214903917 + 11;
                        target = table[(next_random >> 16) % table_size];
                        if (target == 0) target = next_random % (vocab_size - 1) + 1;
                        if (target == word) continue;
                        label = 0;
                    }
                    l2 = target * layer1_size;
                    f = 0;
                    for (c = 0; c < layer1_size; c++) f += neu1[c] * syn1neg[c + l2];
                    if (f > MAX_EXP) g = (label - 1) * alpha;
                    else if (f < -MAX_EXP) g = (label - 0) * alpha;
                    else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                    for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
                    for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * neu1[c];
                }
            // hidden -> in
            for (a = b; a < window * 2 + 1 - b; a++) if (a != window) {
                    c = sentence_position - window + a;
                    if (c < 0) continue;
                    if (c >= sentence_length) continue;
                    last_word = sen[c];
                    if (last_word == -1) continue;
                    for (c = 0; c < layer1_size; c++) syn0[c + last_word * layer1_size] += neu1e[c];
                }
        } else {  //train skip-gram*/
        for (a = b; a < window * 2 + 1 - b; a++) if (a != window) {
                c = sentence_position - window + a;
                if (c < 0) continue;
                if (c >= sentence_length) continue;
                last_word = sen[c];
                if (last_word == -1) continue;
                l1 = last_word * layer1_size;
                for (c = 0; c < layer1_size; c++) neu1e[c] = 0;
                // HIERARCHICAL SOFTMAX
                /*
                if (hs) for (d = 0; d < vocab[word].codelen; d++) {
                        f = 0;
                        l2 = vocab[word].point[d] * layer1_size;
                        // Propagate hidden -> output
                        for (c = 0; c < layer1_size; c++) f += syn0[c + l1] * syn1[c + l2];
                        if (f <= -MAX_EXP) continue;
                        else if (f >= MAX_EXP) continue;
                        else f = expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))];
                        // 'g' is the gradient multiplied by the learning rate
                        g = (1 - vocab[word].code[d] - f) * alpha;
                        // Propagate errors output -> hidden
                        for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1[c + l2];
                        // Learn weights hidden -> output
                        for (c = 0; c < layer1_size; c++) syn1[c + l2] += g * syn0[c + l1];
                    }*/
                // NEGATIVE SAMPLING
                if (negative > 0) for (d = 0; d < negative + 1; d++) {
                        if (d == 0) {
                            target = word;
                            label = 1;
                        } else {
                            next_random = next_random * (unsigned long long)25214903917 + 11;
                            target = table[(next_random >> 16) % table_size];
                            long long vocab_size=v.GetVocabSize();
                            if (target == 0) target = next_random % (vocab_size - 1) + 1;
                            if (target == word) continue;
                            label = 0;
                        }
                        l2 = target * layer1_size;
                        f = 0;
                        for (c = 0; c < layer1_size; c++) f += syn0[c + l1] * syn1neg[c + l2];
                        if (f > MAX_EXP) g = (label - 1) * alpha;
                        else if (f < -MAX_EXP) g = (label - 0) * alpha;
                        else g = (label - expTable[(int)((f + MAX_EXP) * (EXP_TABLE_SIZE / MAX_EXP / 2))]) * alpha;
                        for (c = 0; c < layer1_size; c++) neu1e[c] += g * syn1neg[c + l2];
                        for (c = 0; c < layer1_size; c++) syn1neg[c + l2] += g * syn0[c + l1];
                    }
                // Learn weights input -> hidden
                for (c = 0; c < layer1_size; c++) syn0[c + l1] += neu1e[c];
            }
        //  }
        sentence_position++;
        if (sentence_position >= sentence_length) {
            sentence_length = 0;
            continue;
        }
    }
    fclose(fi);
    //free(neu1);
    free(neu1e);
    pthread_exit(NULL);
}


void SkgNeg::TrainModel() {
    FILE *fin = fopen(train_file, "rb");
    if (fin == NULL) {
        printf("ERROR: training data file not found!\n");
        exit(1);
    }
    fseek(fin, 0, SEEK_END);
    file_size = ftell(fin);
    fclose(fin);
    long a, b, c, d;
    FILE *fo;
    pthread_t *pt = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    if (pt == NULL) {
        fprintf(stderr, "cannot allocate memory for threads\n");
        exit(1);
    }
    printf("Starting training using file %s\n", train_file);
    starting_alpha = alpha;
    /*
    if (read_vocab_file[0] != 0) ReadVocab(); else LearnVocabFromTrainFile();
    if (save_vocab_file[0] != 0) SaveVocab();
    if (output_file[0] == 0) return;
    InitNet();
    if (negative > 0) InitUnigramTable();
    */

    start = clock();
    InitUnigramTable();

    Para param[num_threads];
    for (a = 0; a < num_threads; a++) {param[a].th=this;param[a].id=a;}

    for (a = 0; a < num_threads; a++) pthread_create(&pt[a], NULL, BasicTrainModelThread,&param[a]);
    //for (a = 0; a < num_threads; a++) pthread_create(&pt[a], NULL, TrainModelThread, (void *)a);
    for (a = 0; a < num_threads; a++) pthread_join(pt[a], NULL);

    fo = fopen(output_file, "wb");
    if (fo == NULL) {
        fprintf(stderr, "Cannot open %s: permission denied\n", output_file);
        exit(1);
    }
   // if (classes == 0) {
        // Save the word vectors
        long long vocab_size=v.GetVocabSize();
        fprintf(fo, "%lld %lld\n", vocab_size, layer1_size);
        for (a = 0; a < vocab_size; a++) {
            //if (vocab[a].word != NULL) {
            if ( v.GetVocab()[a].GetWord()!= NULL) {
                // fprintf(fo, "%s ", vocab[a].word);
                fprintf(fo,"%s ",v.GetVocab()[a].GetWord());
            }
            if (binary) for (b = 0; b < layer1_size; b++) fwrite(&syn0[a * layer1_size + b], sizeof(real), 1, fo);
            else for (b = 0; b < layer1_size; b++) fprintf(fo, "%lf ", syn0[a * layer1_size + b]);
            fprintf(fo, "\n");
        }
    /*} else {
        // Run K-means on the word vectors
        int clcn = classes, iter = 10, closeid;
        int *centcn = (int *)malloc(classes * sizeof(int));
        if (centcn == NULL) {
            fprintf(stderr, "cannot allocate memory for centcn\n");
            exit(1);
        }
        int *cl = (int *)calloc(vocab_size, sizeof(int));
        real closev, x;
        real *cent = (real *)calloc(classes * layer1_size, sizeof(real));
        for (a = 0; a < vocab_size; a++) cl[a] = a % clcn;
        for (a = 0; a < iter; a++) {
            for (b = 0; b < clcn * layer1_size; b++) cent[b] = 0;
            for (b = 0; b < clcn; b++) centcn[b] = 1;
            for (c = 0; c < vocab_size; c++) {
                for (d = 0; d < layer1_size; d++) {
                    cent[layer1_size * cl[c] + d] += syn0[c * layer1_size + d];
                    centcn[cl[c]]++;
                }
            }
            for (b = 0; b < clcn; b++) {
                closev = 0;
                for (c = 0; c < layer1_size; c++) {
                    cent[layer1_size * b + c] /= centcn[b];
                    closev += cent[layer1_size * b + c] * cent[layer1_size * b + c];
                }
                closev = sqrt(closev);
                for (c = 0; c < layer1_size; c++) cent[layer1_size * b + c] /= closev;
            }
            for (c = 0; c < vocab_size; c++) {
                closev = -10;
                closeid = 0;
                for (d = 0; d < clcn; d++) {
                    x = 0;
                    for (b = 0; b < layer1_size; b++) x += cent[layer1_size * d + b] * syn0[c * layer1_size + b];
                    if (x > closev) {
                        closev = x;
                        closeid = d;
                    }
                }
                cl[c] = closeid;
            }
        }
        // Save the K-means classes
        for (a = 0; a < vocab_size; a++) fprintf(fo, "%s %d\n", vocab[a].word, cl[a]);
        free(centcn);
        free(cent);
        free(cl);
    }*/
    fclose(fo);
   // free(table);
    free(pt);

}


void SkgNeg::DeleteSkgNeg() {
    //DestroyNet
    if (syn0 != NULL) {
        free(syn0);
    }
    /*
    if (syn1 != NULL) {
        free(syn1);
    }
     */
    if (syn1neg != NULL) {
        free(syn1neg);
    }

    free(table);
    free(expTable);
}
