# README #

## File Organization ##

* **main.cpp**: main function
* **myG.h, myG.cpp**: a class for graph management
* **file.h, file.cpp**: implementation of file I/O
* **common.h, debug.h, graph.h**: some global definitions

## How to Use the Code? ##

* First, use "make" to get the executable file "index.out".
* Then, run "./index.out dataFile indexFile" to construct an index for the graph before update.
* "dataFile" is the graph data file. "indexFile" is the output index file.
* Example:
*       ./index.out graph.txt graph.index
* The per-vertex preprocessing stage uses OpenMP CPU parallelism. Set
  `OMP_NUM_THREADS` to control the number of worker threads; for example:
*       OMP_NUM_THREADS=8 ./index.out graph.txt graph.index

## Data Format ##

* The file "dataFile" containing the graph information is of the following format:

    u_1 v_1
    u_2 v_2
    ...

where (u_i, v_i) is the edge in the graph.
