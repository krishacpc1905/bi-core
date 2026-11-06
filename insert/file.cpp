/***************
input file output file function
****************/

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common.h"
#include "myG.h"
#include "file.h"

int g_iOffset = 1;
#if 0
/*****************
input:
        char *szFileName
        myG &oMpG
description:
        read edge list
******************/
int file_readG(char *pcFile, myG &oMpG)
{
    FILE *fp = NULL;
    P_SIZE iTempX = 0;
    P_SIZE iTempY = 0;
    char szBuffer[ONE_LINE_BUFFER] = {0};
    int res = 0;

    DEBUG_ASSERT(NULL != pcFile);

    fp = fopen(pcFile, "rt");
    if (NULL == fp)
    {
        printf("error no file: %s\n", pcFile);
        DEBUG_ASSERT(0);
    }
    oMpG.m_iMaxUId = 0;
    oMpG.m_iMaxVId = 0;
    while (fgets(szBuffer, ONE_LINE_BUFFER - 1, fp) != NULL)
    {
        res = sscanf(szBuffer, "%lld %lld", &iTempX, &iTempY);
        iTempX = g_iOffset + iTempX;
        iTempY = g_iOffset + iTempY;

        DEBUG_ASSERT(2 == res);

        if (iTempX > oMpG.m_iMaxUId)
        {
            oMpG.m_iMaxUId = iTempX;
            oMpG.m_vUAdj.resize(oMpG.m_iMaxUId + 1);
        }
        oMpG.m_vUAdj[iTempX].push_back(iTempY);
        if (iTempY > oMpG.m_iMaxVId)
        {
            oMpG.m_iMaxVId = iTempY;
            oMpG.m_vVAdj.resize(oMpG.m_iMaxVId + 1);
        }
        oMpG.m_vVAdj[iTempY].push_back(iTempX);

    }
    fclose(fp);
    return 0;
}
#endif
/*****************
input:
        char *szFileName
        vector<pair<P_SIZE, P_SIZE> > &vQuery
description:
        read inserted edge list
******************/
int file_query(char *pcFile, vector<pair<P_SIZE, P_SIZE> > &vQuery)
{
    FILE *fp = NULL;
    P_SIZE iTempX = 0;
    P_SIZE iTempY = 0;
    char szBuffer[ONE_LINE_BUFFER] = {0};
    int res = 0;

    DEBUG_ASSERT(NULL != pcFile);

    fp = fopen(pcFile, "rt");
    if (NULL == fp)
    {
        printf("error no file: %s\n", pcFile);
        DEBUG_ASSERT(0);
    }
    while (fgets(szBuffer, ONE_LINE_BUFFER - 1, fp) != NULL)
    {
        res = sscanf(szBuffer, "%lld %lld", &iTempX, &iTempY);
        iTempX = g_iOffset + iTempX;
        iTempY = g_iOffset + iTempY;

        ASSERT_MSG(2 == res, "ERROR res: "<<res<<" line: "<<szBuffer<<endl);

        vQuery.push_back(pair<P_SIZE, P_SIZE>(iTempX, iTempY));

    }
    fclose(fp);
    return 0;
}
/*****************
input:
        int fd
        char *pcData
        long long llTotoalSize
output:
        none
description:
        write data to file
******************/
int fileBatchSave(int fd, char *pcData, long long llTotoalSize)
{
    uint32_t uMaxBuffer = 100000000;
    long long llCurPos = 0;
    long long uStep = llTotoalSize;
    while (llCurPos < llTotoalSize)
    {
        if (uStep <= uMaxBuffer)
        {
            long long ret = write(fd, pcData + llCurPos, uStep * sizeof(char));
            DEBUG_ASSERT(uStep == ret);
            llCurPos += uStep;

            uStep = uStep < (llTotoalSize - llCurPos)? uStep : (llTotoalSize - llCurPos);
        }
        else
        {
            uStep = uMaxBuffer;
            DEBUG_ASSERT(1 < uStep);
        }
    }
    return 0;
}
/*****************
input:
        myG &oMpG
        char *szGPath
output:
        none
description:
        save truss decomposition
******************/
int file_saveBitIndex(myG &oMpG, char *pcFile)
{
    char sFirstLine[100] = {0};
    char *pcData = NULL;
    long long llIndexCnt = 0;

    /* write */
    int fd = open(pcFile, O_CREAT|O_WRONLY|O_TRUNC, 0644);

    // count index
    for (P_SIZE u = 0; u <= oMpG.m_iMaxUId - g_iOffset; ++u)
    {
        if (oMpG.m_vAdj[1][u + g_iOffset].empty())
        {
            continue;
        }
        llIndexCnt += oMpG.m_vIndexA[1][u + g_iOffset].size() + oMpG.m_vIndexB[1][u + g_iOffset].size();
        DEBUG_ASSERT(oMpG.m_vIndexA[1][u + g_iOffset].size() == oMpG.m_vK[1][u + g_iOffset].k - 1);
        DEBUG_ASSERT(oMpG.m_vIndexB[1][u + g_iOffset].size() == oMpG.m_vK[1][u + g_iOffset].k - 1);
    }
    for (P_SIZE v = 0; v <= oMpG.m_iMaxVId - g_iOffset; ++v)
    {
        if (oMpG.m_vAdj[0][v + g_iOffset].empty())
        {
            continue;
        }
        llIndexCnt += oMpG.m_vIndexA[0][v + g_iOffset].size() + oMpG.m_vIndexB[0][v + g_iOffset].size();
        DEBUG_ASSERT(oMpG.m_vIndexA[0][v + g_iOffset].size() == oMpG.m_vK[0][v + g_iOffset].k - 1);
        DEBUG_ASSERT(oMpG.m_vIndexB[0][v + g_iOffset].size() == oMpG.m_vK[0][v + g_iOffset].k - 1);
    }

    // head
    sprintf(sFirstLine, "U,%lld\nV,%lld\nE,%lld\na,%d\nb,%d\nk,%d\nl,%d\nDU,%lld\nDV,%lld\nI,%lld\n",
            oMpG.m_iMaxUId - g_iOffset,
            oMpG.m_iMaxVId - g_iOffset,
            oMpG.m_iMaxEId,
            oMpG.m_iMaxA,
            oMpG.m_iMaxB,
            oMpG.m_iMaxK,
            oMpG.m_iMaxL,
            oMpG.m_iUMaxD,
            oMpG.m_iVMaxD,
            llIndexCnt);
    fileBatchSave(fd, sFirstLine, 100);

    // first block: nodes
    /* fill data */
    long long llTotoalSize = (oMpG.m_iMaxUId - g_iOffset + oMpG.m_iMaxVId - g_iOffset + 2) * sizeof(SaveNode);
    //DEBUG_PRINTF("FILE first size: %d\n", oMpG.m_iMaxUId - g_iOffset + oMpG.m_iMaxVId - g_iOffset + 2);
    pcData = (char *)malloc(llTotoalSize);
    DEBUG_ASSERT(NULL != pcData);
    SaveNode *pstData = (SaveNode *)pcData;
    for (P_SIZE u = 0; u <= oMpG.m_iMaxUId - g_iOffset; ++u)
    {
        pstData[u].d = oMpG.m_vAdj[1][u + g_iOffset].size();
        pstData[u].k = oMpG.m_vK[1][u + g_iOffset].k;
        pstData[u].l = oMpG.m_vK[1][u + g_iOffset].l;
        pstData[u].ld = oMpG.m_vK[1][u + g_iOffset].d;
        //DEBUG_PRINTF("FILE u: %d index: %d a: %d b: %d\n", u, u, pstData[u].a, pstData[u].b);
    }
    for (P_SIZE v = 0; v <= oMpG.m_iMaxVId - g_iOffset; ++v)
    {
        pstData[v + oMpG.m_iMaxUId - g_iOffset + 1].d = oMpG.m_vAdj[0][v + g_iOffset].size();
        pstData[v + oMpG.m_iMaxUId - g_iOffset + 1].k = oMpG.m_vK[0][v + g_iOffset].k;
        pstData[v + oMpG.m_iMaxUId - g_iOffset + 1].l = oMpG.m_vK[0][v + g_iOffset].l;
        pstData[v + oMpG.m_iMaxUId - g_iOffset + 1].ld = oMpG.m_vK[0][v + g_iOffset].d;
        /*DEBUG_PRINTF("FILE v: %d index: %d a: %d b: %d\n", v, v + oMpG.m_iMaxUId - g_iOffset + 1,
                     pstData[v + oMpG.m_iMaxUId - g_iOffset + 1].a, pstData[v + oMpG.m_iMaxUId - g_iOffset + 1].b);*/
    }

    fileBatchSave(fd, pcData, llTotoalSize);
    free(pcData);
    pcData = NULL;
    pstData = NULL;
    //DEBUG_PRINTF("FILE first size: %d\n", llTotoalSize);

    // second block: index 4 types
    long long llIndexCur = 0;
    /* fill data */
    llTotoalSize = llIndexCnt * sizeof(TPST_INDEX);
    //DEBUG_PRINTF("FILE second size: %d\n", llTotoalSize);
    pcData = (char *)malloc(llTotoalSize);
    //DEBUG_PRINTF("FILE second alloc: %p\n", pcData);
    DEBUG_ASSERT(NULL != pcData);
    TPST_INDEX *pstIndex = (TPST_INDEX *)pcData;
    for (P_SIZE u = 0; u <= oMpG.m_iMaxUId - g_iOffset; ++u)
    {
        for (K_SIZE k = 0; k < oMpG.m_vIndexA[1][u + g_iOffset].size(); ++k)
        {
            pstIndex[llIndexCur + k] = oMpG.m_vIndexA[1][u + g_iOffset][k];
        }
        llIndexCur += oMpG.m_vIndexA[1][u + g_iOffset].size();
        //DEBUG_PRINTF("FILE U index size: %d\n", oMpG.m_vIndexU[u + g_iOffset].size());
		for (K_SIZE k = 0; k < oMpG.m_vIndexB[1][u + g_iOffset].size(); ++k)
        {
            pstIndex[llIndexCur + k] = oMpG.m_vIndexB[1][u + g_iOffset][k];
        }
        llIndexCur += oMpG.m_vIndexB[1][u + g_iOffset].size();
    }
    for (P_SIZE v = 0; v <= oMpG.m_iMaxVId - g_iOffset; ++v)
    {
        for (K_SIZE k = 0; k < oMpG.m_vIndexA[0][v + g_iOffset].size(); ++k)
        {
            pstIndex[llIndexCur + k] = oMpG.m_vIndexA[0][v + g_iOffset][k];
        }
        llIndexCur += oMpG.m_vIndexA[0][v + g_iOffset].size();
        //DEBUG_PRINTF("FILE V index size: %d\n", oMpG.m_vIndexV[v + g_iOffset].size());
        for (K_SIZE k = 0; k < oMpG.m_vIndexB[0][v + g_iOffset].size(); ++k)
        {
            pstIndex[llIndexCur + k] = oMpG.m_vIndexB[0][v + g_iOffset][k];
        }
        llIndexCur += oMpG.m_vIndexB[0][v + g_iOffset].size();
    }

    ASSERT_MSG(llIndexCur == llIndexCnt, "ERROR: "<<llIndexCur<<" "<<llIndexCnt);
    fileBatchSave(fd, pcData, llTotoalSize);
    free(pcData);
    pcData = NULL;
    pstIndex = NULL;
    //DEBUG_PRINTF("FILE second size: %d count: %d\n", llTotoalSize, llIndexCnt);

    // third block: edges
    long long llECur = 0;
    /* fill data */
    llTotoalSize = oMpG.m_iMaxEId * sizeof(P_SIZE);
    pcData = (char *)malloc(llTotoalSize);
    DEBUG_ASSERT(NULL != pcData);
    P_SIZE *pstE = (P_SIZE *)pcData;
    for (P_SIZE u = 0; u <= oMpG.m_iMaxUId - g_iOffset; ++u)
    {
        for (P_SIZE k = 0; k < oMpG.m_vAdj[1][u + g_iOffset].size(); ++k)
        {
            pstE[llECur + k] = oMpG.m_vAdj[1][u + g_iOffset][k] - g_iOffset;
            //DEBUG_PRINTF("FILE third index: %d v: %d\n", llECur + k, pstE[llECur + k] + g_iOffset);
        }
        llECur += oMpG.m_vAdj[1][u + g_iOffset].size();
    }

    DEBUG_ASSERT(llECur == oMpG.m_iMaxEId);
    fileBatchSave(fd, pcData, llTotoalSize);
    free(pcData);
    pcData = NULL;
    pstE = NULL;
    //DEBUG_PRINTF("FILE third size: %d\n", llTotoalSize);

    close(fd);
    return 0;
}

/*****************
input:
        int fd
        char *pcData
        long long llTotoalSize
output:
        none
description:
        read data from file
******************/
int fileBatchRead(int fd, char *pcData, long long llTotoalSize)
{
    uint32_t uMaxBuffer = 100000000;
    long long llCurPos = 0;
    long long uStep = llTotoalSize;
    while (llCurPos < llTotoalSize)
    {
        if (uStep <= uMaxBuffer)
        {
            long long ret = read(fd, pcData + llCurPos, uStep * sizeof(char));
            DEBUG_ASSERT(uStep == ret);
            llCurPos += uStep;

            uStep = uStep < (llTotoalSize - llCurPos)? uStep : (llTotoalSize - llCurPos);
        }
        else
        {
            uStep = uMaxBuffer;
            DEBUG_ASSERT(1 < uStep);
        }
    }
    return 0;
}
/*****************
input:
        char *szFileName
        myG &mpG
description:
        read file and fill the list g_initG
******************/
int file_readBitIndex(myG &oMpG, char *pcFile)
{
    char sFirstLine[100] = {0};
    char *pcData = NULL;

    /* read */
    int fd = open(pcFile, O_RDONLY);
    DEBUG_ASSERT(-1 != fd);
    fileBatchRead(fd, sFirstLine, 100);
    P_SIZE iU, iV, iM, iDU, iDV, iIndex;
    int iA, iB, iK, iL;
    int ret = sscanf(sFirstLine, "U,%lld\nV,%lld\nE,%lld\na,%d\nb,%d\nk,%d\nl,%d\nDU,%lld\nDV,%lld\nI,%lld\n", &iU, &iV, &iM, &iA, &iB, &iK, &iL, &iDU, &iDV, &iIndex);
    ASSERT_MSG(10 == ret, "ERROR "<<ret);
    DEBUG_PRINTF("FILE u_max: %lld v_max: %lld\n", iU, iV);
    cout << "FILE u_max: " << iU << " v_max: " << iV << endl;
    DEBUG_PRINTF("FILE u_max: %lld v_max: %lld\n", iU, iV);

    /* fill data */
    /* first line */
    oMpG.m_iMaxUId = iU + g_iOffset;
    oMpG.m_iMaxVId = iV + g_iOffset;
    oMpG.m_iMaxEId = iM;
    oMpG.m_iMaxA = iA;
    oMpG.m_iMaxB = iB;
    oMpG.m_iMaxK = iK;
    oMpG.m_iMaxL = iL;
    oMpG.m_iUMaxD = iDU;
    oMpG.m_iVMaxD = iDV;

    // first block: nodes
    long long llTotoalSize = (iU + iV + 2) * sizeof(SaveNode);
    long long llIndexCnt = 0;

    pcData = (char *)malloc(llTotoalSize);
    ASSERT_MSG(NULL != pcData, "Error malloc size: " << llTotoalSize<< " u: " << iU << " v: " << iV << " item: " << sizeof(SaveNode));
    SaveNode *pstData = (SaveNode *)pcData;

    fileBatchRead(fd, pcData, llTotoalSize);

    //oMpG.m_vMax[1].resize(iU + g_iOffset + 1);
    //oMpG.m_vMax[0].resize(iV + g_iOffset + 1);
    oMpG.m_vD[1].resize(iU + g_iOffset + 1);
    oMpG.m_vD[0].resize(iV + g_iOffset + 1);
    oMpG.m_vAdj[1].resize(iU + g_iOffset + 1);
    oMpG.m_vAdj[0].resize(iV + g_iOffset + 1);
    oMpG.m_vIndexA[1].resize(iU + g_iOffset + 1);
    oMpG.m_vIndexA[0].resize(iV + g_iOffset + 1);
    oMpG.m_vIndexB[1].resize(iU + g_iOffset + 1);
    oMpG.m_vIndexB[0].resize(iV + g_iOffset + 1);
    oMpG.m_vK[1].resize(iU + g_iOffset + 1);
    oMpG.m_vK[0].resize(iV + g_iOffset + 1);
    oMpG.m_vbA[1].resize(iU + g_iOffset + 1);
    oMpG.m_vbA[0].resize(iV + g_iOffset + 1);
    for (P_SIZE u = 0; u <= iU; ++u)
    {
        //DEBUG_PRINTF("FILE u: %d a: %d b: %d\n", u, pstData[u].a, pstData[u].b);
        //oMpG.m_vMax[1][u + g_iOffset] = pair<K_SIZE, K_SIZE>(pstData[u].a, pstData[u].b);
        oMpG.m_vD[1][u + g_iOffset] = pstData[u].d;
        if (pstData[u].d > 0)
        {
            oMpG.m_vbA[1][u + g_iOffset] = true;
            oMpG.m_vAdj[1][u + g_iOffset].resize(pstData[u].d);
            oMpG.m_vK[1][u + g_iOffset] = {pstData[u].k, pstData[u].ld, pstData[u].l};
            if (pstData[u].k > 1)
            {
                // index rooted at (2, 2)-core
                oMpG.m_vIndexA[1][u + g_iOffset].resize(pstData[u].k - 1);
                oMpG.m_vIndexB[1][u + g_iOffset].resize(pstData[u].k - 1);
                llIndexCnt += 2 * pstData[u].k - 2;
            }
        }
        else
        {
            oMpG.m_vbA[1][u + g_iOffset] = false;
        }
    }
    for (P_SIZE v = 0; v <= iV; ++v)
    {
        //DEBUG_PRINTF("FILE v: %d pos: %d a: %d b: %d\n", v, iU + 1 + v, pstData[iU + 1 + v].a, pstData[iU + 1 + v].b);
        //oMpG.m_vMax[0][v + g_iOffset] = pair<K_SIZE, K_SIZE>(pstData[iU + 1 + v].a, pstData[iU + 1 + v].b);
        oMpG.m_vD[0][v + g_iOffset] = pstData[iU + 1 + v].d;
        if (pstData[iU + 1 + v].d > 0)
        {
            oMpG.m_vbA[0][v + g_iOffset] = true;
            oMpG.m_vAdj[0][v + g_iOffset].reserve(pstData[iU + 1 + v].d);
            oMpG.m_vK[0][v + g_iOffset] = {pstData[iU + 1 + v].k, pstData[iU + 1 + v].ld, pstData[iU + 1 + v].l};
            if (pstData[iU + 1 + v].k > 1)
            {
                // index rooted at (2, 2)-core
                oMpG.m_vIndexA[0][v + g_iOffset].resize(pstData[iU + 1 + v].k - 1);
                oMpG.m_vIndexB[0][v + g_iOffset].resize(pstData[iU + 1 + v].k - 1);
                llIndexCnt += 2 * pstData[iU + 1 + v].k - 2;
            }
        }
        else
        {
            oMpG.m_vbA[0][v + g_iOffset] = false;
        }
    }

    free(pcData);
    pcData = NULL;
    pstData = NULL;
    //DEBUG_PRINTF("FILE read first end size: %d\n", llTotoalSize);

    // second block: index
    ASSERT_MSG(llIndexCnt == iIndex, "ERROR count: "<<llIndexCnt<<" read: "<<iIndex<<endl);
    long long llIndexCur = 0;
    llTotoalSize = llIndexCnt * sizeof(TPST_INDEX);
    pcData = (char *)malloc(llTotoalSize);
    DEBUG_ASSERT(NULL != pcData);
    TPST_INDEX *pstIndex = (TPST_INDEX *)pcData;

    fileBatchRead(fd, pcData, llTotoalSize);
    //DEBUG_PRINTF("FILE read second success\n");

    for (P_SIZE u = 0; u <= iU; ++u)
    {
        if (!oMpG.m_vbA[1][u + g_iOffset])
        {
            continue;
        }
        if (oMpG.m_vK[1][u + g_iOffset].k < 2)
        {
            continue;
        }
        for (int k = 0; k < oMpG.m_vIndexA[1][u + g_iOffset].size(); ++k)
        {
            oMpG.m_vIndexA[1][u + g_iOffset][k] = pstIndex[llIndexCur + k];
        }
        llIndexCur += oMpG.m_vIndexA[1][u + g_iOffset].size();
        for (int k = 0; k < oMpG.m_vIndexB[1][u + g_iOffset].size(); ++k)
        {
            oMpG.m_vIndexB[1][u + g_iOffset][k] = pstIndex[llIndexCur + k];
        }
        llIndexCur += oMpG.m_vIndexB[1][u + g_iOffset].size();
    }
    for (P_SIZE v = 0; v <= iV; ++v)
    {
        if (!oMpG.m_vbA[0][v + g_iOffset])
        {
            continue;
        }
        if (oMpG.m_vK[0][v + g_iOffset].k < 2)
        {
            continue;
        }
        for (int k = 0; k < oMpG.m_vIndexA[0][v + g_iOffset].size(); ++k)
        {
            oMpG.m_vIndexA[0][v + g_iOffset][k] = pstIndex[llIndexCur + k];
        }
        llIndexCur += oMpG.m_vIndexA[0][v + g_iOffset].size();
        for (int k = 0; k < oMpG.m_vIndexB[0][v + g_iOffset].size(); ++k)
        {
            oMpG.m_vIndexB[0][v + g_iOffset][k] = pstIndex[llIndexCur + k];
        }
        llIndexCur += oMpG.m_vIndexB[0][v + g_iOffset].size();
    }
    ASSERT_MSG(llIndexCur == llIndexCnt, "ERROR size: "<<llIndexCur<<" "<<llIndexCnt);

    free(pcData);
    pcData = NULL;
    pstIndex = NULL;
    //DEBUG_PRINTF("FILE read second end size: %d count: %d\n", llTotoalSize, llIndexCnt);

    // third block: edges
    long long llECur = 0;
    llTotoalSize = iM * sizeof(P_SIZE);
    pcData = (char *)malloc(llTotoalSize);
    DEBUG_ASSERT(NULL != pcData);
    P_SIZE *pstE = (P_SIZE *)pcData;

    fileBatchRead(fd, pcData, llTotoalSize);
    //DEBUG_PRINTF("FILE read third success\n");

    for (P_SIZE u = 0; u <= iU; ++u)
    {
        if (!oMpG.m_vbA[1][u + g_iOffset])
        {
            continue;
        }
        //DEBUG_PRINTF("FILE third u: %d\n", u);
        DEBUG_ASSERT(oMpG.m_vAdj[1][u + g_iOffset].size() == oMpG.m_vD[1][u + g_iOffset]);
        for (P_SIZE k = 0; k < oMpG.m_vD[1][u + g_iOffset]; ++k)
        {
            //DEBUG_PRINTF("FILE third index: %d v: %d\n", llECur + k, pstE[llECur + k] + g_iOffset);
            oMpG.m_vAdj[1][u + g_iOffset][k] = pstE[llECur + k] + g_iOffset;
            oMpG.m_vAdj[0][pstE[llECur + k] + g_iOffset].push_back(u + g_iOffset);
        }
        llECur += oMpG.m_vD[1][u + g_iOffset];
    }
    //DEBUG_PRINTF("FILE sort adj\n");
    for (P_SIZE v = 0; v <= iV; ++v)
    {
        if (!oMpG.m_vbA[0][v + g_iOffset])
        {
            continue;
        }
        sort(oMpG.m_vAdj[0][v + g_iOffset].begin(), oMpG.m_vAdj[0][v + g_iOffset].end());
    }
    DEBUG_ASSERT(llECur == iM);

    free(pcData);
    pcData = NULL;
    pstE = NULL;

    //DEBUG_PRINTF("FILE read third end size: %d\n", llTotoalSize);

    close(fd);
    return 0;
}
