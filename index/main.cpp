/***************
main cpp
the code for truss incremental maintenance in paper
"Efficient Star-based Truss Maintenance on Dynamic Graphs".
****************/
#include "common.h"
#include "myG.h"
#include "file.h"

// graph object
myG g_baseG;

extern long g_lCopyTime;
extern long g_lPeelTime;

/*****************
input:
        index file name
        star file
        query file
        output index file
        output truss file
description:
        main function
******************/
int main(int argc, char *argv[])
{
	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;
	long lIndexTime = 0;
	long lInitTime = 0;

    if (2 > argc)
    {
        printf("argc: %d\n", argc);
        DEBUG_ASSERT(0);
    }

    char *pcData = argv[1];

    printf("start read graph\n");
    file_readG(pcData, g_baseG);
    printf("start init\n");
    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
    g_baseG.init();
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
    lInitTime = lCurTime - lStartTime;
    printf("init done\n");
    printf("U_max: %d V_max: %d E_max: %d\n",
           g_baseG.m_iMaxUId, g_baseG.m_iMaxVId, g_baseG.m_iMaxEId);

    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
    g_baseG.index();
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
    lIndexTime += lCurTime - lStartTime;
    //g_baseG.print();
    printf("K_max: %d L_max: %d\n",
           g_baseG.m_iMaxK, g_baseG.m_iMaxL);

    /* save result */
    if (2 < argc)
    {
        file_saveBitIndex(g_baseG, argv[2]);
    }

	printf("Copy time: %.3f ms\n",
        g_lCopyTime / 1000.0);
	printf("Preprocessing time: %.3f ms\n",
        lInitTime / 1000.0);
	printf("Peeling time: %.3f ms\n",
        g_lPeelTime / 1000.0);
	printf("Indexing time: %.3f ms\n",
        lIndexTime / 1000.0);

    return 0;
}
