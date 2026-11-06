# README #

## File Organization ##

* **main.cpp**: main function
* **myG.h, myG.cpp**: a class for graph management
* **file.h, file.cpp**: implementation of file I/O
* **common.h, debug.h, graph.h**: some global definitions

## How to Use the Code? ##

* First, use "make" to get the executable file "delete.out".
* Then, run "./delete.out indexFile deleteFile" to update the index of the graph after update.
* "indexFile" is the output index file. "deleteFile" is edges to be removed from the graph.
* Example:
*       ./delete.out graph.index delete.txt
* The default batch update is sequential. To update adjacency lists in
  parallel by endpoint-owned bulk removals before the existing sequential
  index maintenance, run:
*       OMP_NUM_THREADS=8 BI_CORE_PARALLEL=1 ./delete.out graph.index delete.txt
* Add an output index path as the third argument to save the updated index.

## Data Format ##

* The file "deleteFile" containing the graph information is of the following format:

    u_1 v_1
    u_2 v_2
    ...

where (u_i, v_i) is the edge to be removed from the graph.
