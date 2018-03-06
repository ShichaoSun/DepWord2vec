# DepWord2vec
train word2vec in dependency tree

## Requirement
cmake

## Running
- Stage 1: Get training data(dependency tree of [wiki corpus](https://dumps.wikimedia.org/enwiki/latest/enwiki-latest-pages-articles.xml.bz2))

- Stage 2: Download vocab file ([vocab](https://drive.google.com/open?id=1j2_6t9Z9LQjeGX2nvUc5oAa9GNI0nL3D))

- Stage 3: Get the executable binary file
```
mkdir build
cd build
cmake ..
make
```
or you can directly download the executable binary file [Dep2vec](https://drive.google.com/open?id=1aj3EtPnnSBZj_jWVmPGBVZB1WDdHXgp3)

- Stage 4: Run

```
./Dep2vec default_dep2vec_config.json
```
