# Efficient Semantic Similarity Search over Spatio-textual Data

## Table of Contents

  * [Introduction](#markdown-header-introduction)
  * [Dependencies](#markdown-header-dependencies)
  * [Build](#markdown-header-build)
  * [Run](#markdown-header-run)
  * [Contact](#markdown-header-contact)


------------------------------------
## Introduction

This repository contains the source code for reproducing for our 
paper Efficient Semantic Similarity Search over Spatio-textual Data. 

The code is in two parts:
* Preprocessing: Takes as input file with geo-location and text (.e., from tweets), and creates indexes as decribed in our paper.
* Querying: Uses preprocessed data files and indexes from proprocessing for efficient querying using the algorithms described in the paper.

If you use our code, please cite:

> George S. Theodoropoulos, Kjetil Nørvåg, and Christos Doulkeridis, Efficient Semantic Similarity Search over Spatio-textual Data, Proceedings of EDBT'2024, Paestum, Italy, March 2024.

------------------------------------
## Preprocessing

See description in the "indexing" folder. 

------------------------------------
## Querying

### Build

This project has been set up to use `CMake` for an out-of-source build.

The code is using:
* the Boost C++ libraries: https://www.boost.org/
* the [src/rtree/RTree.h](/src/rtree/RTree.h) file is from https://github.com/nushoin/RTree

To build the code in _debug_ mode, execute the following instructions:
> cd CSSI/querying
>
> mkdir bin-debug
>
> cd bin-debug
>
> cmake -B../bin-debug  -S../src/ -DCMAKE_BUILD_TYPE=Debug
>
> make

To build the code in _release_ mode, execute the following instructions:
> cd CSSI/querying
>
> mkdir bin-release
>
> cd bin-release
>
> cmake -B../bin-release  -S../src/ -DCMAKE_BUILD_TYPE=Release
>
> make

You might need to install the following packages: g++, cmake, and libboost-all-dev.

### Run

By running the executable `sstss` with no arguments gives
a list of allowed options.

An example execution
can be the following (using the toy-size dataset under "indexing"):

> ./sstss --workloaddir ../../indexing/twitter_dataset --idxdir ../../indexing/twitter_indexes --m 2 --k 10 --a 0.5 --algorithm "scan rtree s2r csi3 csia3" --rndq 10

In this example, the five algorithms are executed for 10 query points, with given parameters ("a" is the "lambda" in the paper, csi3 and csia3 are the CSSI cnd CSSIA algorithms in the paper). The running times and statistics are shown in the console and appended in the `results_log.txt` file.

For each algorithm, the average of the running times of all the queries, and the sum of the scores from all the queries, are calculated and shown in the results with query point `0` and query index `0`.

## License

This code is published under the MIT License.

## Acknowledgment

Some of the overall experimental framework is based on code written by Stella Maropaki and
Sean Chester for the paper "Diversifying Top-k Point-of-Interest Queries via Collective Social Reach", https://github.com/stellamaro/diversify-topk-pois.

## Contact

Feel free to submit any questions, comments, or corrections to @giorgostheo (code for preprocessing) or @noervaag (code for querying).
