
# CSSI

The CSSI repo

## Quickstart

To quickly get a result using the toy dataset of 10K tweets that is supplied, do the following:

```bash
cd querying/
./make_code.sh -m r
```

This will use the presupplied indexes and data to run a small set of runs that include all the algorithms that are presented. The result will be return in stdout.

## Introduction

The code is in two parts:
* Indexing: Takes as input file with geo-location and text (from tweets), and creates indexes as described in our paper.
* Querying: Uses data files and indexes created by the indexing step for efficient querying using the algorithms described in the paper.


## Indexing

See description in the "indexing" folder. 


## Querying

### Dependencies

None for the moment.

### Build

This project has been set up to use `CMake` for an out-of-source build.

The code is using:
* the Boost C++ libraries: https://www.boost.org/
* the [src/rtree/RTree.h](/src/rtree/RTree.h) file is from https://github.com/nushoin/RTree


To build the code in _debug_ mode, execute the following instructions:
> mkdir bin-debug
>
> cd bin-debug
>
> cmake -B../bin-debug  -S../src/ -DCMAKE_BUILD_TYPE=Debug
>
> make

To build the code in _release_ mode, execute the following instructions:
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

For each algorithm, the median of the running times of all the queries, and the sum of the scores from all the queries, are calculated and shown in the results with query point `0` and query index `0`.

### Scripts

The currect project includes the `make_code.sh` script that can be used to build and run
the code. It uses two input parameters:

  Parameter     | Description
----------------|--------------
 -m             | build mode: `r` for _release_ and `d` for _debug_

This script builds the code in the _debug_ or _release_ modes from scratch every time, and then runs the code using the example above.

Example use of the script:
> ./make_code.sh -m r

In this example, the code is build in _release_ mode and the example execution is run.

## License

This code is published under the MIT License.
