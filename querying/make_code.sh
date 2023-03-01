#!/usr/bin/env bash
# ./make_code.sh
# -m d / r  -> d=debug r=release

while getopts r:m: option
do
	case "${option}"
	in
    r) RUN=${OPTARG};;
	m) MODE=${OPTARG};;
	esac
done

CMAKE=""
FOLDER=""

if [[ "$MODE" = "d" ]]; then
	FOLDER="bin-debug"
	CMAKE="cmake -B../$FOLDER  -S../src/ -DCMAKE_BUILD_TYPE=Debug"
elif [[ "$MODE" = "r" ]]; then
	FOLDER="bin-release"
	CMAKE="cmake -B../$FOLDER -S../src/ -DCMAKE_BUILD_TYPE=Release"
fi

echo ${FOLDER}
echo ${CMAKE}

rm -rf "$FOLDER"
mkdir ${FOLDER}
cd ${FOLDER}/
${CMAKE}
make

 ./sstss --workloaddir ../../indexing/twitter_dataset --idxdir ../../indexing/twitter_indexes --m 2 --k 10 --a 0.5 --algorithm "scan rtree s2r csi3 csia3" --rndq 10
