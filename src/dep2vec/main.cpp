//
// Created by bruce on 7/13/16.
//

#include <stdio.h>
#include <iostream>
#include <fstream>
#include "DepSkgNeg.h"
#include <rapidjson/document.h>

int main(int argc, char **argv) {

    Vocab vocab;
    char train_file[MAX_STRING];
    char default_config[MAX_STRING]="/home/bruce/ClionProjects/DepWord2vec/default_dep2vec_config.json";
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

    DepSkgNeg depskgneg(vocab);
    depskgneg.SetTrainfile(train_file);

    if (config.HasMember("save_vocab")) {
        std::string save_vocab = config["save_vocab"].GetString();
        vocab.SaveVocab(save_vocab.c_str());
    }

    if (config.HasMember("layer1_size"))
        depskgneg.Setlayer1_size(config["layer1_size"].GetInt());

    if (config.HasMember("debug_mode"))
        depskgneg.SetDebugmode(config["debug_mode"].GetInt());

    if (config.HasMember("binary"))
        depskgneg.SetBinary(config["binary"].GetInt());

    if (config.HasMember("iter"))
        depskgneg.SetIter(config["iter"].GetInt());

    if (config.HasMember("window"))
        depskgneg.SetWindow(config["window"].GetInt());

    if (config.HasMember("sample"))
        depskgneg.SetSample(config["sample"].GetFloat());

    if (config.HasMember("negative"))
        depskgneg.SetNegative(config["negative"].GetInt());

    if (config.HasMember("threads"))
        depskgneg.SetNumthread(config["threads"].GetInt());

    if(config.HasMember("alpha"))
        depskgneg.SetAlpha(config["alpha"].GetFloat());

    depskgneg.TrainModel();

    if (config.HasMember("output_file")) {
        std::string output_file=config["output_file"].GetString();
        depskgneg.SaveWordVectors(output_file.c_str());
    }

    return 0;
}