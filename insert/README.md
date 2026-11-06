# README #

## File Organization ##

* **main.cpp**: main function
* **myG.h, myG.cpp**: a class for graph management
* **file.h, file.cpp**: implementation of file I/O
* **common.h, debug.h, graph.h**: some global definitions

## How to Use the Code? ##

* First, use "make" to get the executable file "insert.out".
* Then, run "./insert.out indexFile insertFile" to update the index of the graph after update.
* "indexFile" is the output index file. "insertFile" is edges to be inserted to the graph.
* Example:
*       ./insert.out graph.index insert.txt
* The default batch update is sequential. To update adjacency lists in
  parallel by endpoint-owned bulk merges before the existing sequential index
  maintenance, run:
*       OMP_NUM_THREADS=8 BI_CORE_PARALLEL=1 ./insert.out graph.index insert.txt
* Add an output index path as the third argument to save the updated index.

## Data Format ##

* The file "insertFile" containing the graph information is of the following format:

    u_1 v_1
    u_2 v_2
    ...

where (u_i, v_i) is the edge to be inserted to the graph.
