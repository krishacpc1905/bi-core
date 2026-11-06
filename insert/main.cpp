/***************
main cpp
the code for truss incremental maintenance in paper
"Efficient Star-based Truss Maintenance on Dynamic Graphs".
****************/
#include "common.h"
#include "myG.h"
#include "file.h"
#include <cstdlib>

extern long g_simpleTime;
extern long g_findTime;
extern long g_maintainTime0;
extern long g_maintainTime1;
extern long g_maintainTime2;
extern long g_maintainUpgradeCnt;
extern long g_maintainSortTime;
extern long g_maintainProcessTime;
extern long g_maintainDoneTime;
extern long g_checkIndexTime;
extern long g_estimateTime;

// graph object
myG g_baseG;

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
	long lQueryTime = 0;

    if (3 > argc)
    {
        printf("argc: %d\n", argc);
        DEBUG_ASSERT(0);
    }

    char *pcData = argv[1];
    char *pcQuery = argv[2];
    vector<pair<P_SIZE, P_SIZE> > vQuery;

    printf("start read graph\n");
    file_readBitIndex(g_baseG, pcData);
    printf("U_max: %d V_max: %d E_max: %d\n",
           g_baseG.m_iMaxUId, g_baseG.m_iMaxVId, g_baseG.m_iMaxEId);

    file_query(pcQuery, vQuery);

    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
    /*for (int i = 0; i < vQuery.size(); ++i)
    {
        g_baseG.edge_insert(vQuery[i].first, vQuery[i].second);
    }*/
    const char *parallelEnv = getenv("BI_CORE_PARALLEL");
    if (parallelEnv != NULL && parallelEnv[0] == '1' && parallelEnv[1] == '\0')
    {
        g_baseG.batch_insert_parallel(vQuery);
    }
    else
    {
        g_baseG.batch_insert(vQuery);
    }
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
    lQueryTime += lCurTime - lStartTime;

    /* save result */
    if (3 < argc)
    {
        file_saveBitIndex(g_baseG, argv[3]);
    }

	printf("g_simpleTime: %.3f ms\n",
        g_simpleTime / 1000.0);
	printf("g_findTime: %.3f ms\n",
        g_findTime / 1000.0);
	printf("g_maintainUpgradeCnt: %lld\n",
        g_maintainUpgradeCnt);
	printf("g_maintainTime: %.3f ms %.3f ms %.3f ms\n",
        g_maintainTime0 / 1000.0, g_maintainTime1 / 1000.0, g_maintainTime2 / 1000.0);
	printf("g_maintainSortTime: %.3f ms\n",
        g_maintainSortTime / 1000.0);
	printf("g_maintainProcessTime: %.3f ms\n",
        g_maintainProcessTime / 1000.0);
	printf("g_maintainDoneTime: %.3f ms\n",
        g_maintainDoneTime / 1000.0);
	printf("g_checkIndexTime: %.3f ms\n",
        g_checkIndexTime / 1000.0);
	printf("g_estimateTime: %.3f ms\n",
        g_estimateTime / 1000.0);
	printf("Total query time: %.3f ms\n",
        lQueryTime / 1000.0);

    return 0;
}
