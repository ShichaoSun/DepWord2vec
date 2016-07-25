#include <stdio.h>
#include <iostream>
#include <fstream>
#include "SkgNeg.h"
#include <rapidjson/document.h>



/*
int ArgPos(char *str, int argc, char **argv) {
    int a;
    for (a = 1; a < argc; a++) if (!strcmp(str, argv[a])) {
            if (a == argc - 1) {
                printf("Argument missing for %s\n", str);
                exit(1);
            }
            return a;
        }
    return -1;
}
*/

int main(int argc, char **argv) {
    Vocab vocab;
    char train_file[MAX_STRING];
    char default_config[MAX_STRING]="/home/bruce/ClionProjects/DepWord2vec/default_word2vec_config.json";
    std::ifstream in(default_config, std::ios::in);
    std::istreambuf_iterator<char> beg(in), end;
    std::string jsondata(beg, end);
    rapidjson::Document config;
    config.Parse(jsondata.c_str());

    if (config.HasMember("train_file")) {
        std::string trainfile = config["train_file"].GetString();
        strcpy(train_file, trainfile.c_str());
        if (config.HasMember("read_vocab")) {
            std::string read_vocab = config["read_vocab"].GetString();
            vocab.ReadVocab(read_vocab.c_str());
        }
        else
            vocab.LearnVocabFromTrainFile(train_file);
    } else {
        printf("train_file can't be empty!");
        return 0;
    }

    if (config.HasMember("min_count"))
        vocab.SetMincount(config["min_count"].GetInt());

    SkgNeg skgneg(vocab);
    skgneg.SetTrainfile(train_file);

    if (config.HasMember("save_vocab")) {
        std::string save_vocab = config["save_vocab"].GetString();
        vocab.SaveVocab(save_vocab.c_str());
    }

    if (config.HasMember("layer1_size"))
        skgneg.Setlayer1_size(config["layer1_size"].GetInt());

    if (config.HasMember("iter"))
        skgneg.SetIter(config["iter"].GetInt());

    if (config.HasMember("debug_mode"))
        skgneg.SetDebugmode(config["debug_mode"].GetInt());

    if (config.HasMember("binary"))
        skgneg.SetBinary(config["binary"].GetInt());

    if (config.HasMember("window"))
        skgneg.SetWindow(config["window"].GetInt());

    if (config.HasMember("iter"))
        skgneg.SetIter(config["iter"].GetInt());

    if (config.HasMember("sample"))
        skgneg.SetSample(config["sample"].GetFloat());

    if (config.HasMember("negative"))
        skgneg.SetNegative(config["negative"].GetInt());

    if (config.HasMember("threads"))
        skgneg.SetNumthread(config["threads"].GetInt());

    if(config.HasMember("alpha"))
        skgneg.SetAlpha(config["alpha"].GetFloat());

    skgneg.TrainModel();

    if (config.HasMember("output_file")) {
        std::string output_file=config["output_file"].GetString();
        skgneg.SaveWordVectors(output_file.c_str());
    }
    /*int i,j;
    if (argc == 1) {
        printf("WORD VECTOR FROM DEPENDENCY TREE\n\n");
        printf("Options:\n");
        printf("Parameters for training:\n");
        printf("\t-train <file>\n");
        printf("\t\tUse text data from <file> to train the model\n");
        printf("\t-output <file>\n");
        printf("\t\tUse <file> to save the resulting word vectors / word clusters\n");
        printf("\t-size <int>\n");
        printf("\t\tSet size of word vectors; default is 100\n");
        printf("\t-window <int>\n");
        printf("\t\tSet max skip length between words; default is 5\n");
        printf("\t-sample <float>\n");
        printf("\t\tSet threshold for occurrence of words. Those that appear with higher frequency");
        printf(" in the training data will be randomly down-sampled; default is 0 (off), useful value is 1e-5\n");
        //printf("\t-hs <int>\n");
        //printf("\t\tUse Hierarchical Softmax; default is 1 (0 = not used)\n");
        printf("\t-negative <int>\n");
        printf("\t\tNumber of negative examples; default is 0, common values are 5 - 10 (0 = not used)\n");
        printf("\t-threads <int>\n");
        printf("\t\tUse <int> threads (default 1)\n");
        printf("\t-min-count <int>\n");
        printf("\t\tThis will discard words that appear less than <int> times; default is 5\n");
        printf("\t-alpha <float>\n");
        printf("\t\tSet the starting learning rate; default is 0.025\n");
        printf("\t-iter <int>\n");
        printf("\t\tRun more training iterations (default 5)\n");
        //printf("\t-classes <int>\n");
        //printf("\t\tOutput word classes rather than word vectors; default number of classes is 0 (vectors are written)\n");
        printf("\t-debug <int>\n");
        printf("\t\tSet the debug mode (default = 2 = more info during training)\n");
        printf("\t-binary <int>\n");
        printf("\t\tSave the resulting vectors in binary moded; default is 0 (off)\n");
        printf("\t-save-vocab <file>\n");
        printf("\t\tThe vocabulary will be saved to <file>\n");
        printf("\t-read-vocab <file>\n");
        printf("\t\tThe vocabulary will be read from <file>, not constructed from the training data\n");
        //printf("\t-cbow <int>\n");
        //printf("\t\tUse the continuous back of words model; default is 0 (skip-gram model)\n");
        printf("\nExamples:\n");
        printf("./word2vec -train data.txt -output vec.txt -debug 2 -size 200 -window 5 -sample 1e-4 -negative 5 -hs 0 -binary 0 \n\n");
        return 0;
    }

    Vocab vocab;
    char train_file[MAX_STRING];
    if ((i = ArgPos((char *)"-train", argc, argv)) > 0) {
        strcpy(train_file,argv[i + 1]);
        if ((j = ArgPos((char *) "-read-vocab", argc, argv)) <= 0)
            vocab.LearnVocabFromTrainFile(argv[i + 1]);
        else
            vocab.ReadVocab(argv[j+1]);
    } else{
        printf("-train can't empty!");
        return 0;
    }

    if ((i = ArgPos((char *)"-min-count", argc, argv)) > 0)
        vocab.SetMincount(atoi(argv[i + 1]));

    SkgNeg skgneg(vocab);
    skgneg.SetTrainfile(train_file);

    if ((i = ArgPos((char *)"-save-vocab", argc, argv)) > 0)
        vocab.SaveVocab(argv[i + 1]);


    if ((i = ArgPos((char *)"-size", argc, argv)) > 0)
        skgneg.Setlayer1_size(atoi(argv[i + 1]));


    if ((i = ArgPos((char *)"-debug", argc, argv)) > 0)
        skgneg.SetDebugmode(atoi(argv[i + 1]));

    if ((i = ArgPos((char *)"-binary", argc, argv)) > 0)
        skgneg.SetBinary(atoi(argv[i + 1]));

    //if ((i = ArgPos((char *)"-cbow", argc, argv)) > 0) cbow = atoi(argv[i + 1]);
    if ((i = ArgPos((char *)"-alpha", argc, argv)) > 0)
        skgneg.SetAlpha(atof(argv[i + 1]));


    if ((i = ArgPos((char *)"-window", argc, argv)) > 0)
        skgneg.SetWindow(atoi(argv[i + 1]));

    if ((i = ArgPos((char *)"-sample", argc, argv)) > 0)
        skgneg.SetSample(atof(argv[i + 1]));

    //if ((i = ArgPos((char *)"-hs", argc, argv)) > 0) hs = atoi(argv[i + 1]);
    if ((i = ArgPos((char *)"-negative", argc, argv)) > 0)
        skgneg.SetNegative(atoi(argv[i + 1]));

    if ((i = ArgPos((char *)"-threads", argc, argv)) > 0)
        skgneg.SetNumthread(atoi(argv[i + 1]));


    //if ((i = ArgPos((char *)"-classes", argc, argv)) > 0) classes = atoi(argv[i + 1]);
    //vocab = (struct vocab_word *)calloc(vocab_max_size, sizeof(struct vocab_word));
    //vocab_hash = (int *)calloc(vocab_hash_size, sizeof(int));
    //expTable = (real *)malloc((EXP_TABLE_SIZE + 1) * sizeof(real));
    //if (expTable == NULL) {
      //  fprintf(stderr, "out of memory\n");
      //  exit(1);
    //}
   // for (i = 0; i < EXP_TABLE_SIZE; i++) {
    //    expTable[i] = exp((i / (real)EXP_TABLE_SIZE * 2 - 1) * MAX_EXP); // Precompute the exp() table
    //    expTable[i] = expTable[i] / (expTable[i] + 1);                   // Precompute f(x) = x / (x + 1)
   // }
    //TrainModel();
   // DestroyNet();
    //free(vocab_hash);
    //free(expTable);

    skgneg.TrainModel();

    if ((i = ArgPos((char *)"-output", argc, argv)) > 0)
        skgneg.SaveWordVectors(argv[i + 1]);*/

    return 0;
}