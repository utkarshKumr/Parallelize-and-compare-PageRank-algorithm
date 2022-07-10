# Parallelize and compare PageRank on OpenMP & Pthreads

## Description
In this project, we mainly focus on the speedup and efficient utilization of the cache for the PageRank algorithm by reducing the cache misses. A edge-centric sorted implementation of parallel PageRank has been implemented using OpenMP is in the folder `openmp/pagerank`. For PThreads it is in `pthreads/`

> Used `crunchy1` machine and `GCC 9.2` for compilation and test.

## Quick Steps to execute
### OpenMP implementation
- Go to `openmp/pagerank` folder
- Using `gcc-9.2` on `crunchy1` execute
```
$ make
$ ./pr ../data/binary_web-Google.graph 32
```
- In above 2nd param is name of input file and 3rd is numThreads

### pThreads implementation
##### Parallel
- Go to `pthreads/pagerankParallel` folder
- Using `gcc-9.2` on `crunchy1` execute
```
$ make
$ ./pagerankPThreads.out ../data/web-Google.txt 1000000 0.0000001 16
```
- In above 2nd param is name of input file and 3rd is Vertices and 4th is threshold and 5th is threads

##### Sequential
- Go to `pthreads/pagerankSerial` folder
- Using `gcc-9.2` on `crunchy1` execute
```
$ make
$ ./pagerankSerial.out ../data/web-Google.txt 1000000 0.0000001
```
- In above 2nd param is name of input file and 3rd is Vertices and 4th is threashold



.
.
## Detail Steps to execute with another/custom input
### OpenMP implementation
`data` folder has the 2 datasets we have used in this project. One can extract a dataset, doing:
```
$ cd openmp/data/
$ gzip -d <dataFile name>
```

The datasets are submitted in binary format.
For example, if you want to use the dataset 'web-Google', run the following command:
```
$ cd openmp/data/
$ gzip -d simple_web-Google.graph.gz
```
Now the generated *.graph can be used as input.

Now, to compile and run the PageRank program, go to the directory containing Makefile and execute the make command. This will create an executable called `pr`:
```
$ cd openmp/pagerank/
$ make
```
Next, run the following command to launch the program on a particular graph using a certain number of threads:
```
$ ./pr <path to graph> <number of threads> 
```
For example, run the following command to launch the program on the web-Google graph using 16 threads:
```
$ ./pr ../data/binary_web-Google.graph 16 
```

Scheduling policy and chunk size can be changed by modifying the `page_rank.cpp` file.


### Pthreads

The `data` folder contains the datasets used in this project. To extract any dataset, run the following command:
```
$ cd pthreads/data/
$ gzip -d <dataFile name>
```
For example, if you want to use the dataset 'web-Google.txt.gz', run the following command:
```
$ cd pthreads/data/
$ gzip -d web-Google.txt.gz
```
This will create a text file called `web-Google.txt`.

##### Parallel

```
$ cd pthreads/pagerankParallel/
$ make
```
Next, run the following command to launch the program on a particular graph using a certain number of threads:
```
$ ./pagerankPThreads.out <path to data file> <number of vertices in the graph> <convergance threshold of pageRank> <number of threads>
```

##### Serial
```
$ cd pthreads/pagerankSerial/
$ make
```
Next, run the following command to launch the program on a particular graph:
```
$ ./pagerankSerial.out <path to data file> <number of vertices in the graph> <convergance threshold of pageRank>
```
For example, run the following command to launch the program on the web-Google graph:
```
$ ./pagerank_serial ../data/web-Google.txt 1000000 0.0000001
```