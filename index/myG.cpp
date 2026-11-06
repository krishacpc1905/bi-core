/***************
my class G
****************/

#include "common.h"
#include "myG.h"

long g_lCopyTime;
long g_lPeelTime;

/*****************
input:
        none
description:
        init object
        init stage
******************/
myG::myG()
{
    m_iMaxUId = 0;
    m_iMaxVId = 0;
    m_iMaxEId = 0;
}
/*****************
input:
        none
description:
        init after add nodes
******************/
int myG::init()
{
    DEBUG_PRINTF("INIT start U max: %d\n", m_iMaxUId);
    m_iUMaxD = 0;
    m_iMaxEId = 0;
    E_SIZE edgeCount = 0;
    P_SIZE maxUDegree = 0;
    m_vD[1].resize(m_iMaxUId + 1);
    m_vbA[1].resize(m_iMaxUId + 1);
    DEBUG_ASSERT(m_iMaxUId == m_vAdj[1].size() - 1);
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1024) reduction(+:edgeCount) reduction(max:maxUDegree)
#endif
    for (P_SIZE vid = 1; vid <= m_iMaxUId; ++vid)
    {
        //DEBUG_PRINTF("INIT u: %I64d\n", vid);
        /*sort(m_vUAdj[vid].begin(), m_vUAdj[vid].end(),
            [](const TPST_ADJ& e1, const TPST_ADJ& e2) {
            return e1.iPid < e2.iPid;
            });
        auto atTp = unique(m_vUAdj[vid].begin(), m_vUAdj[vid].end(),
            [](const TPST_ADJ& e1, const TPST_ADJ& e2) {
            return e1.iPid == e2.iPid;
            });
        m_vUAdj[vid].erase(atTp, m_vUAdj[vid].end());*/
        COMMON_UNIQUE(m_vAdj[1][vid]);
        if (!m_vAdj[1][vid].empty())
        {
            m_vD[1][vid] = m_vAdj[1][vid].size();
            edgeCount += m_vD[1][vid];
            maxUDegree = max(maxUDegree, static_cast<P_SIZE>(m_vAdj[1][vid].size()));
        }
    }
    m_iMaxEId = edgeCount;
    m_iUMaxD = maxUDegree;
    for (P_SIZE vid = 1; vid <= m_iMaxUId; ++vid)
    {
        m_vbA[1][vid] = !m_vAdj[1][vid].empty();
    }
    DEBUG_PRINTF("INIT start V max: %d\n", m_iMaxVId);
    P_SIZE maxVDegree = 0;
    m_vD[0].resize(m_iMaxVId + 1);
    m_vbA[0].resize(m_iMaxVId + 1);
    DEBUG_ASSERT(m_iMaxVId == m_vAdj[0].size() - 1);
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1024) reduction(max:maxVDegree)
#endif
    for (P_SIZE vid = 1; vid <= m_iMaxVId; ++vid)
    {
        //DEBUG_PRINTF("INIT v: %d\n", vid);
        /*sort(m_vVAdj[vid].begin(), m_vVAdj[vid].end(),
            [](const TPST_ADJ& e1, const TPST_ADJ& e2) {
            return e1.iPid < e2.iPid;
            });
        auto atTp = unique(m_vVAdj[vid].begin(), m_vVAdj[vid].end(),
            [](const TPST_ADJ& e1, const TPST_ADJ& e2) {
            return e1.iPid == e2.iPid;
            });
        m_vVAdj[vid].erase(atTp, m_vVAdj[vid].end());*/
        COMMON_UNIQUE(m_vAdj[0][vid]);
        if (!m_vAdj[0][vid].empty())
        {
            m_vD[0][vid] = m_vAdj[0][vid].size();
            maxVDegree = max(maxVDegree, static_cast<P_SIZE>(m_vAdj[0][vid].size()));
        }
    }
    m_iVMaxD = maxVDegree;
    for (P_SIZE vid = 1; vid <= m_iMaxVId; ++vid)
    {
        m_vbA[0][vid] = !m_vAdj[0][vid].empty();
    }
    DEBUG_PRINTF("INIT d_uMax: %d d_vMax: %d\n", m_iUMaxD, m_iVMaxD);

    return 0;
}
/*****************
input:
        myG &oInitG
description:
        copy map
        calculate stage
******************/
myG::~myG()
{
    m_iMaxUId = 0;
    m_iMaxVId = 0;
    m_iMaxEId = 0;

    //m_pvPNeCnt->clear();

    //delete m_pvPNeCnt;
}

/*****************
input:
        vector<vector<P_SIZE> > &vAdjA
        vector<vector<P_SIZE> > &vAdjB
        vector<P_SIZE> &vRemainA
        vector<P_SIZE> &vRemainB
        vector<P_SIZE> &vDA
        vector<P_SIZE> &vDB
        K_SIZE iDesKA
        K_SIZE iDesKB
        vector<TPST_LAYER> &vAL
        vector<TPST_LAYER> &vBL
description:
        peel edges not in (iDesKA, iDesKB)-core
******************/
int myG::peelingUnit(vector<vector<P_SIZE> > &vAdjA, vector<vector<P_SIZE> > &vAdjB, vector<P_SIZE> &vRemainA, vector<P_SIZE> &vRemainB, vector<P_SIZE> &vDA, vector<P_SIZE> &vDB, K_SIZE iDesKA, K_SIZE iDesKB, vector<TPST_LAYER> &vAL, vector<TPST_LAYER> &vBL)
{
    int iTotal = 0;
    if (vRemainA.empty() && vRemainB.empty())
    {
        return 0;
    }

    // peeling by layers
    L_SIZE iLayer = 1;
    vARmQ.clear();
    vBRmQ.clear();
    vARmQBack.clear();
    vBRmQBack.clear();

    vAL.clear();
    vBL.clear();

    // init removed
    for (P_SIZE iTpP : vRemainA)
    {
        if (vDA[iTpP] < iDesKA)
        {
            vARmQBack.push_back(iTpP);
        }
    }

    for (P_SIZE iTpP : vRemainB)
    {
        if (vDB[iTpP] < iDesKB)
        {
            vBRmQBack.push_back(iTpP);
        }
    }
    iTotal = vRemainA.size() + vRemainB.size();

    while (iTotal)
    {
        // remove nodes
        vARmQ.swap(vARmQBack);
        vBRmQ.swap(vBRmQBack);

        if (vARmQ.empty() && vBRmQ.empty())
        {
            break;
        }

        // batch save
        for (P_SIZE liA : vARmQ)
        {
            vAL.push_back({liA, (K_SIZE)(iDesKB - 1), (K_SIZE)vDA[liA], iLayer});
        }
        for (P_SIZE liB : vBRmQ)
        {
            vBL.push_back({liB, (K_SIZE)(iDesKB - 1), (K_SIZE)vDB[liB], iLayer});
        }

        // batch remove
        while (!(vARmQ.empty()))
        {
            P_SIZE liA = vARmQ.back();
            vARmQ.pop_back();
            --iTotal;
            vDA[liA] = 0;
            //DEBUG_PRINTF("PEELING remove (%d, %d) a_%d layer: %d\n", iDesKA, iDesKB, liA, iLayer);
            for (P_SIZE liB : vAdjA[liA])
            {
                if (0 < vDB[liB])
                {
                    vDB[liB]--;
                    if (vDB[liB] == iDesKB - 1)
                    {
                        vBRmQBack.push_back(liB);
                    }
                }
            }
        }
        while (!(vBRmQ.empty()))
        {
            P_SIZE liB = vBRmQ.back();
            vBRmQ.pop_back();
            --iTotal;
            vDB[liB] = 0;
            //DEBUG_PRINTF("PEELING remove (%d, %d) b_%d layer: %d\n", iDesKA, iDesKB, liB, iLayer);
            for (P_SIZE liA : vAdjB[liB])
            {
                if (0 < vDA[liA])
                {
                    vDA[liA]--;
                    if (vDA[liA] == iDesKA - 1)
                    {
                        vARmQBack.push_back(liA);
                    }
                }
            }
        }
        ++iLayer;
    }

    --iLayer;
    m_iMaxL = max(m_iMaxL, iLayer);

    // update edges
    if (iTotal != vRemainA.size() + vRemainB.size())
    {
        // exist edges removed
        vNextA.clear();
        vNextB.clear();
        for (P_SIZE iTpP : vRemainA)
        {
            if (vDA[iTpP] >= iDesKA)
            {
                vNextA.push_back(iTpP);
            }
        }

        for (P_SIZE iTpP : vRemainB)
        {
            if (vDB[iTpP] >= iDesKB)
            {
                vNextB.push_back(iTpP);
            }
        }
        vRemainA.swap(vNextA);
        vRemainB.swap(vNextB);
    }
    return 0;
}
/*****************
input:
        vector<vector<P_SIZE> > &vAdjA
        vector<vector<P_SIZE> > &vAdjB
        vector<P_SIZE> &vRemainA
        vector<P_SIZE> &vRemainB
        vector<P_SIZE> &vDA
        vector<P_SIZE> &vDB
        K_SIZE iKA
        vector<TPST_LAYER> &vAL
        vector<TPST_LAYER> &vBL
description:
        fix iKA, peel iKB
******************/
int myG::peeling(vector<vector<P_SIZE> > &vAdjA, vector<vector<P_SIZE> > &vAdjB, vector<P_SIZE> &vRemainA, vector<P_SIZE> &vRemainB, vector<P_SIZE> &vDA, vector<P_SIZE> &vDB, K_SIZE iKA, vector<TPST_LAYER> &vAL, vector<TPST_LAYER> &vBL)
{
    int iTotal = 0;
    if (vRemainA.empty() && vRemainB.empty())
    {
        return 0;
    }
    K_SIZE iKB = 2;
    L_SIZE iLayer = 1;
    // peeling by layers
    vARmQ.clear();
    vBRmQ.clear();
    vARmQBack.clear();
    vBRmQBack.clear();
    vAL.clear();
    vBL.clear();
    vBin.clear();
    P_SIZE iBinSize = *max_element(vDB.begin(), vDB.end()) + 1;

    vBin.resize(iBinSize);

    for (P_SIZE iTpP : vRemainA)
    {
        if (vDA[iTpP] < iKA)
        {
            vARmQBack.push_back(iTpP);
        }
    }

    for (P_SIZE iTpP : vRemainB)
    {
        if (vDB[iTpP] < iKB)
        {
            vBRmQBack.push_back(iTpP);
        }
        else
        {
            vBin[vDB[iTpP]].push_back(iTpP);
        }
    }

    iTotal = vRemainA.size() + vRemainB.size();

    while (iTotal)
    {
        if (vARmQBack.empty() && vBRmQBack.empty())
        {
            int iBinMax = min((P_SIZE)iKB, iBinSize);
            for (int i = iKB - 1; i < iBinMax; ++i)
            {
                for (int j = 0; j < vBin[i].size(); ++j)
                {
                    P_SIZE iCurP = vBin[i][j];
                    if (0 < vDB[iCurP])
                    {
                        vBRmQBack.push_back(iCurP);
                    }
                }
            }
        }
        // remove nodes
        vARmQ.swap(vARmQBack);
        vBRmQ.swap(vBRmQBack);
        //DEBUG_PRINTF("PEELING current k: (%d, %d) layer: %d remove %d remain: %d\n", iKA, iKB, iLayer, vARmQ.size() + vBRmQ.size(), iTotal);

        if (vARmQ.empty() && vBRmQ.empty())
        {
            ++iKB;
            m_iMaxL = max(m_iMaxL, (L_SIZE)(iLayer-1));
            iLayer = 1;
            DEBUG_ASSERT(iKB <= iBinSize);
            continue;
        }

        while (!(vARmQ.empty()))
        {
            P_SIZE liA = vARmQ.back();
            vARmQ.pop_back();
            --iTotal;
            vAL.push_back({liA, (K_SIZE)(iKB - 1), (K_SIZE)vDA[liA], iLayer});
            vDA[liA] = -1;
            //DEBUG_PRINTF("PEELING remove (%d, %d) a_%d layer: %d\n", iKA, iKB, liA, iLayer);
            for (P_SIZE liB : vAdjA[liA])
            {
                if (0 < vDB[liB])
                {
                    vDB[liB]--;
                    if (vDB[liB] == iKB - 1)
                    {
                        vBRmQBack.push_back(liB);
                    }
                    else if (vDB[liB] >= iKB)
                    {
                        vBin[vDB[liB]].push_back(liB);
                    }
                }
            }
        }
        while (!(vBRmQ.empty()))
        {
            P_SIZE liB = vBRmQ.back();
            vBRmQ.pop_back();
            if (0 > vDB[liB])
            {
                // removed
                continue;
            }
            --iTotal;
            vBL.push_back({liB, (K_SIZE)(iKB - 1), (K_SIZE)vDB[liB], iLayer});
            vDB[liB] = -1;

            //DEBUG_PRINTF("PEELING remove (%d, %d) b_%d layer: %d\n", iKA, iKB, liB, iLayer);
            for (P_SIZE liA : vAdjB[liB])
            {
                if (0 < vDA[liA])
                {
                    vDA[liA]--;
                    if (vDA[liA] == iKA - 1)
                    {
                        vARmQBack.push_back(liA);
                    }
                }
            }
        }
        ++iLayer;
        //DEBUG_PRINTF("PEELING remove success remain: %d\n", vRemainA.size() + vRemainB.size());
    }

    --iLayer;
    m_iMaxL = max(m_iMaxL, iLayer);

    return iKB - 1;
}

/*****************
input:
        vector<vector<TPST_INDEX> > &vIndex
        vector<TPST_LAYER> &vLayer
        K_SIZE iFixK
        bool bFirst     // true: alpha, false: beta
description:
        save index
******************/
int myG::save(vector<vector<TPST_INDEX> > &vIndex, vector<TPST_LAYER> &vLayer)
{
    for (auto atIt : vLayer)
    {
        vIndex[atIt.p].push_back({atIt.k, atIt.d, atIt.l});
        //DEBUG_PRINTF("SAVE p: %d k: %d, l: %d\n", atIt.p, atIt.k, atIt.l);
    }
    return 0;
}
/*****************
input:
        vector<vector<TPST_INDEX> > &vIndex
        vector<TPST_LAYER> &vLayer
        K_SIZE iFixK
        bool bFirst     // true: alpha, false: beta
description:
        save index
******************/
int myG::save(bool bU, vector<TPST_LAYER> &vLayer)
{
    for (auto atIt : vLayer)
    {
        m_vK[bU][atIt.p] = {atIt.k, atIt.d, atIt.l};
    }
    return 0;
}
/*****************
input:
        none
description:
        index construction
******************/
int myG::index()
{
    vector<P_SIZE> vDU;
    vector<P_SIZE> vDV;
    vector<bool> vbU;
    vector<bool> vbV;
    vector<TPST_LAYER> vUL;
    vector<TPST_LAYER> vVL;

    /*vector<vector<P_SIZE> > vBinU;
    vector<vector<P_SIZE> > vBinV;
    vector<vector<P_SIZE> > vBinUBack;
    vector<vector<P_SIZE> > vBinVBack;*/
    vector<P_SIZE> vRemainU;
    vector<P_SIZE> vRemainV;
    vector<P_SIZE> vRemainUBack;
    vector<P_SIZE> vRemainVBack;

    long ldTrussCnt = 0;

	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;

	/*vBinU.resize(m_iUMaxD + 1);
	vBinV.resize(m_iVMaxD + 1);*/
    vRemainU.reserve(m_iMaxUId + 1);
    vRemainV.reserve(m_iMaxVId + 1);

    // step 1: push bin
    /*P_SIZE liUTotal = 0;
    P_SIZE liVTotal = 0;
    P_SIZE liUTotalBack = 0;
    P_SIZE liVTotalBack = 0;*/
    DEBUG_ASSERT(m_vAdj[1].size() <= m_vD[1].size());
    DEBUG_ASSERT(m_vAdj[1].size() <= m_vbA[1].size());
    for (P_SIZE liN = 1; liN < m_vAdj[1].size(); ++liN)
    {
        if (m_vD[1][liN] <= 0)
        {
            continue;
        }
        //vBinU[m_vDU[liN]].push_back(liN);
        vRemainU.push_back(liN);
        DEBUG_ASSERT(m_vbA[1][liN]);
        //++liUTotal;
    }

    DEBUG_ASSERT(m_vAdj[0].size() <= m_vD[0].size());
    DEBUG_ASSERT(m_vAdj[0].size() <= m_vbA[0].size());
    for (P_SIZE liN = 1; liN < m_vAdj[0].size(); ++liN)
    {
        if (m_vD[0][liN] <= 0)
        {
            continue;
        }
        //vBinV[m_vDV[liN]].push_back(liN);
        vRemainV.push_back(liN);
        DEBUG_ASSERT(m_vbA[0][liN]);
        //++liVTotal;
    }
    //DEBUG_PRINTF("INDEX init queue done\n");

    // step 2: peeling
    m_vIndexA[1].resize(m_iMaxUId + 1);
    m_vIndexA[0].resize(m_iMaxVId + 1);
    m_vIndexB[1].resize(m_iMaxUId + 1);
    m_vIndexB[0].resize(m_iMaxVId + 1);
    m_vK[1].resize(m_iMaxUId + 1);
    m_vK[0].resize(m_iMaxVId + 1);


    K_SIZE iAllK = 1;
    K_SIZE iKA = 1;
    K_SIZE iKB = 1;
    m_iMaxA = 1;
    m_iMaxB = 1;
    while (!vRemainU.empty() || !vRemainV.empty())
    {
        // first (k, k)-core to (k+1, k+1)-core direction
        ldTrussCnt += vRemainU.size() + vRemainV.size();
        //DEBUG_PRINTF("INDEX peeling (%d, %d)->(%d, %d) total: %d %d\n", iAllK, iAllK, iAllK+1, iAllK+1, vRemainU.size(), vRemainV.size());
        gettimeofday(&tv, NULL);
        lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
        peelingUnit(m_vAdj[1], m_vAdj[0], vRemainU, vRemainV, m_vD[1], m_vD[0], iAllK+1, iAllK+1, vUL, vVL);
        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
        g_lPeelTime += lCurTime - lStartTime;
        //DEBUG_PRINTF("INDEX peeling done (%d, %d)->(%d, %d) total: %d %d\n", iAllK, iAllK, iAllK+1, iAllK+1, vRemainU.size(), vRemainV.size());
        save(true, vUL);
        save(false, vVL);

        if (vRemainU.empty() && vRemainV.empty())
        {
            break;
        }
        ++iAllK;

        // second peeling a direction until empty
        iKA = iAllK;

        // backup G and internal results
        gettimeofday(&tv, NULL);
        lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
        /*vBinUBack = vBinU;
        vBinVBack = vBinV;
        liUTotalBack = liUTotal;
        liVTotalBack = liVTotal;*/
        vRemainUBack = vRemainU;
        vRemainVBack = vRemainV;
        vDU = m_vD[1];
        vDV = m_vD[0];
        /*vbU = m_vbU;
        vbV = m_vbV;*/
        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
        g_lCopyTime += lCurTime - lStartTime;

        //DEBUG_PRINTF("INDEX fix a = %d peeling b\n", iKA);
        //DEBUG_PRINTF("INDEX peeling (%d, %d)->(%d, inf) total: %d %d\n", iKA, iAllK, iKA, vRemainUBack.size(), vRemainVBack.size());
        gettimeofday(&tv, NULL);
        lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
        K_SIZE iKBMax = peeling(m_vAdj[1], m_vAdj[0], vRemainUBack, vRemainVBack, vDU, vDV, iKA, vUL, vVL);
        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
        g_lPeelTime += lCurTime - lStartTime;
        //DEBUG_PRINTF("INDEX peeling done (%d, %d)->(%d, %d)\n", iKA, iAllK, iKA, iKBMax);
        //DEBUG_PRINTF("INDEX save (%d, %d)\n", iKA, iKBMax);
        //DEBUG_PRINTF("INDEX save u\n");
        save(m_vIndexA[1], vUL);
        //DEBUG_PRINTF("INDEX save v\n");
        save(m_vIndexA[0], vVL);
        m_iMaxB = max(m_iMaxB, iKBMax);

        // third peeling b direction until empty
        iKB = iAllK;

        // backup G and internal results
        gettimeofday(&tv, NULL);
        lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
        /*vBinUBack = vBinU;
        vBinVBack = vBinV;
        liUTotalBack = liUTotal;
        liVTotalBack = liVTotal;*/
        vRemainUBack = vRemainU;
        vRemainVBack = vRemainV;
        vDU = m_vD[1];
        vDV = m_vD[0];
        /*vbU = m_vbU;
        vbV = m_vbV;*/
        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
        g_lCopyTime += lCurTime - lStartTime;

        //DEBUG_PRINTF("INDEX fix b = %d peeling a\n", iKB);
        //DEBUG_PRINTF("INDEX peeling (%d, %d)->(inf, %d) total: %d %d\n", iAllK, iKB, iKB, vRemainUBack.size(), vRemainVBack.size());
        gettimeofday(&tv, NULL);
        lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
        K_SIZE iKAMax = peeling(m_vAdj[0], m_vAdj[1], vRemainVBack, vRemainUBack, vDV, vDU, iKB, vVL, vUL);
        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
        g_lPeelTime += lCurTime - lStartTime;
        //DEBUG_PRINTF("INDEX peeling done (%d, %d)->(%d, %d)\n", iAllK, iKB, iKAMax, iKB);
        //DEBUG_PRINTF("INDEX save u\n");
        save(m_vIndexB[1], vUL);
        //DEBUG_PRINTF("INDEX save v\n");
        save(m_vIndexB[0], vVL);

        m_iMaxA = max(m_iMaxA, iKAMax);
    }

    m_iMaxK = iAllK;

    DEBUG_PRINTF("INDEX refine index done max: (%d, %d) k max: %d T: %lld\n", m_iMaxA, m_iMaxB, m_iMaxK, ldTrussCnt);

    return 0;
}

/*****************
input:
        none
description:
        init after add nodes
******************/
int myG::print(bool bType, P_SIZE p)
{
    if (bType)
    {
        DEBUG_PRINTF("PRINT U: %d k (%d, %d, %d) [", p, m_vK[1][p].k,  m_vK[1][p].l,  m_vK[1][p].d);
    }
    else
    {
        DEBUG_PRINTF("PRINT V: %d k (%d, %d, %d) [", p, m_vK[0][p].k,  m_vK[0][p].l,  m_vK[0][p].d);
    }

    for (P_SIZE i = 0; i < m_vAdj[bType][p].size(); ++i)
    {
        DEBUG_PRINTF("%d,", m_vAdj[bType][p][i]);
    }
    DEBUG_PRINTF("]\n[");
    for (P_SIZE i = 0; i < m_vIndexA[bType][p].size(); ++i)
    {
        DEBUG_PRINTF("(%d,%d,%d),", m_vIndexA[bType][p][i].k, m_vIndexA[bType][p][i].l, m_vIndexA[bType][p][i].d);
    }
    DEBUG_PRINTF("]\n[");
    for (P_SIZE i = 0; i < m_vIndexB[bType][p].size(); ++i)
    {
        DEBUG_PRINTF("(%d,%d,%d),", m_vIndexB[bType][p][i].k, m_vIndexB[bType][p][i].l, m_vIndexB[bType][p][i].d);
    }
    DEBUG_PRINTF("]\n");

    return 0;
}

/*****************
input:
        none
description:
        init after add nodes
******************/
int myG::print()
{
    DEBUG_PRINTF("PRINT U max: %d\n", m_iMaxUId);
    DEBUG_PRINTF("PRINT V max: %d\n", m_iMaxVId);
    DEBUG_PRINTF("PRINT E max: %d\n", m_iMaxEId);
    DEBUG_PRINTF("PRINT d_U max: %d\n", m_iUMaxD);
    DEBUG_PRINTF("PRINT d_V max: %d\n", m_iVMaxD);
    DEBUG_PRINTF("PRINT alpha max: %d\n", m_iMaxA);
    DEBUG_PRINTF("PRINT beta max: %d\n", m_iMaxB);
    DEBUG_PRINTF("PRINT k max: %d\n", m_iMaxK);
    DEBUG_PRINTF("PRINT l max: %d\n", m_iMaxL);

    for (P_SIZE u = 1; u <= m_iMaxUId; ++u)
    {
        print(true, u);
    }
    for (P_SIZE v = 1; v <= m_iMaxVId; ++v)
    {
        print(false, v);
    }

    return 0;
}







