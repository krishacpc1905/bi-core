/***************
my class G
****************/

#include "common.h"
#include "myG.h"

long g_simpleTime;
long g_findTime;
long g_maintainTime0;
long g_maintainTime1;
long g_maintainTime2;
long g_maintainUpgradeCnt;
long g_maintainSortTime;
long g_maintainProcessTime;
long g_maintainDoneTime;
long g_checkIndexTime;
long g_estimateTime;

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
        vector<vector<P_SIZE> > &vAdj
        int y
description:
        insert y into m_vAdj[x]
        return position
******************/
int myG::adjInsert(vector<P_SIZE> &vAdj, P_SIZE p)
{
    /* binary search position */
    int iSize = vAdj.size();
    int iMin = 0;
    int iMax = iSize;
    int iPos = iSize / 2;
    while (iMin < iMax)
    {
        if (vAdj[iPos] > p)
        {
            iMax = iPos;
            iPos = (iMin + iMax) / 2;
        }
        else if (vAdj[iPos] < p)
        {
            iMin = iPos + 1;
            iPos = (iMin + iMax) / 2;
        }
        else
        {
            /* found */
            return iPos;
        }
    }

    if (iPos < iSize)
    {
        vAdj.insert(vAdj.begin() + iPos, p);
    }
    else
    {
        vAdj.push_back(p);
    }
    return iPos;
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

/*****************
input:
        P_SIZE p
        bool bNodeType
        int iPeelType
        K_SIZE k
description:
        return NULL: not in it; otherwise the point
******************/
TPST_INDEX *myG::findPoint(P_SIZE p, bool bNodeType, int iPeelType, K_SIZE k)
{
    int iKMax = m_vK[bNodeType][p].k;
    if (k > iKMax)
    {
        return NULL;
    }

    if (0 == iPeelType)
    {
        // up
        // index start from 2
        DEBUG_ASSERT(k > 1);
        DEBUG_ASSERT(m_vIndexA[bNodeType][p].size() == iKMax - 1);
        return &(m_vIndexA[bNodeType][p][k-2]);
    }
    else if (1 == iPeelType)
    {
        // right
        // index start from 2
        DEBUG_ASSERT(k > 1);
        DEBUG_ASSERT(m_vIndexB[bNodeType][p].size() == iKMax - 1);
        return &(m_vIndexB[bNodeType][p][k-2]);
    }
    else
    {
        // right up
        DEBUG_ASSERT(iPeelType == 2);
        return &(m_vK[bNodeType][p]);
    }
}

/*****************
input:
        P_SIZE p
        bool bNodeType  //true: u; false: v
        int iPeelType  //0: b; 1: a; 2: k
        K_SIZE a
        K_SIZE b
        L_SIZE l
description:
        return *pRes: -1: x before y; 0: x equal y; 1: x later than y
        return TPST_INDEX *: pointer of node with lower order
******************/
TPST_INDEX *myG::minLayer(P_SIZE x, bool bXType, P_SIZE y, bool bYType, int iPeelType, K_SIZE k, int *pRes)
{
    TPST_INDEX *pstMin = NULL;
    *pRes = 0;
    TPST_INDEX *pstX = findPoint(x, bXType, iPeelType, k);
    TPST_INDEX *pstY = findPoint(y, bYType, iPeelType, k);

    if ((NULL == pstX) && (NULL == pstY))
    {
        // not in it
        DEBUG_ASSERT(0);
        *pRes = 0;
        return NULL;
    }
    else if (NULL == pstX)
    {
        *pRes = -1;
        return NULL;
    }
    else if (NULL == pstY)
    {
        *pRes = 1;
        return NULL;
    }
    else
    {
        if (pstX->k < pstY->k)
        {
            *pRes = -1;
            return pstX;
        }
        else if (pstX->k > pstY->k)
        {
            *pRes = 1;
            return pstY;
        }
        // k equal
        if (pstX->l < pstY->l)
        {
            *pRes = -1;
            return pstX;
        }
        else if (pstX->l > pstY->l)
        {
            *pRes = 1;
            return pstY;
        }
        else
        {
            *pRes = 0;
            return pstX;
        }
    }
}
/*****************
input:
        P_SIZE p
        bool bNodeType  //true: u; false: v
        int iPeelType  //0: b; 1: a; 2: k
        K_SIZE a
        K_SIZE b
        L_SIZE l
description:
        check whether node meets layer requirement
******************/
bool myG::checkLayer(P_SIZE p, bool bNodeType, int iPeelType, K_SIZE iMinK)
{
    TPST_INDEX *pstRes = findPoint(p, bNodeType, iPeelType, iMinK);
    DEBUG_ASSERT(pstRes);

    /*DEBUG_PRINTF("CHECK node: %d, type: %d peel: %d (%d, %d) layer: %d d: %d %d\n",
                 p, bNodeType, iPeelType, iMinK, iBigK, l, iPrevDegree, iDegree);*/
    if (iPeelType == 0)
    {
        K_SIZE a = iMinK;
        K_SIZE b = pstRes->k;
        // (a, b)-core -> (a, b+1)-core
        if (bNodeType)
        {
            //u
            DEBUG_ASSERT(pstRes->l > 1);
            //ASSERT_MSG(iPrevDegree >= a, "CHECK LAYER ERROR p: "<<p<<" type: "<<bNodeType);
            //DEBUG_ASSERT(iDegree < a);
            if (pstRes->d >= a)
            {
                //DEBUG_PRINTF("CHECK false 1\n");
                return false;
            }
        }
        else
        {
            // v
            if (pstRes->l > 1)
            {
                //DEBUG_ASSERT(iPrevDegree > b);
                //DEBUG_ASSERT(iDegree <= b);
                if (pstRes->d > b)
                {
                    //DEBUG_PRINTF("CHECK false 2\n");
                    return false;
                }
            }
            else
            {
                //DEBUG_PRINTF("CHECK node: %d type: %d (%d, %d) degree: %d b: %d\n", p, bNodeType, a, b, pstRes->d, b);
                DEBUG_ASSERT(pstRes->d >= b);
                if (pstRes->d != b)
                {
                    //DEBUG_PRINTF("CHECK false 3 node: %d type: %d (%d, %d) degree: %d b: %d\n", p, bNodeType, a, b, pstRes->d, b);
                    return false;
                }
            }
        }
    }
    else if (iPeelType == 1)
    {
        K_SIZE a = pstRes->k;
        K_SIZE b = iMinK;
        // (a, b)-core -> (a+1, b)-core
        if (bNodeType)
        {
            // u
            if (pstRes->l > 1)
            {
                //ASSERT_MSG(iPrevDegree > a, "CHECK LAYER ERROR p: "<<p<<" type: "<<bNodeType);
                //DEBUG_ASSERT(iDegree <= a);
                if (pstRes->d > a)
                {
                    //DEBUG_PRINTF("CHECK false 4\n");
                    return false;
                }
            }
            else
            {
                ASSERT_MSG(pstRes->d >= a, "CHECK LAYER ERROR p: "<<p<<" type: "<<bNodeType);
                if (pstRes->d != a)
                {
                    //DEBUG_PRINTF("CHECK false 5 p: %d type: %d degree: %d a: %d\n", p, bNodeType, pstRes->d, a);
                    return false;
                }
            }
        }
        else
        {
            // v
            DEBUG_ASSERT(pstRes->l > 1);
            //DEBUG_ASSERT(iPrevDegree >= b);
            //DEBUG_ASSERT(iDegree < b);
            if (pstRes->d >= b)
            {
                //DEBUG_PRINTF("CHECK false 6\n");
                return false;
            }
        }
    }
    else
    {
        // (a, b)-core -> (a+1, b+1)-core
        DEBUG_ASSERT(iPeelType == 2);
        if (pstRes->l > 1)
        {
            //ASSERT_MSG(iPrevDegree > iMinK, "CHECK LAYER ERROR p: "<<p<<" type: "<<bNodeType);
            //DEBUG_ASSERT(iDegree <= a);
            if (pstRes->d > iMinK)
            {
                //DEBUG_PRINTF("CHECK false 7\n");
                return false;
            }
        }
        else
        {
            ASSERT_MSG(pstRes->d >= iMinK, "ERROR node: "<<p<<" type: "<<bNodeType<<" degree: "<<pstRes->d<<" k: "<<iMinK);
            if (pstRes->d != iMinK)
            {
                //DEBUG_PRINTF("CHECK false 8 node: %d type: %d degree: %d k: %d\n", p, bNodeType, pstRes->d, iMinK);
                return false;
            }
        }
    }

    return true;
}

/*****************
input:
        P_SIZE p
        bool bNodeType  //true: u; false: v
        int iPeelType  //0: b; 1: a; 2: k
        K_SIZE a
        K_SIZE b
        L_SIZE l
description:
        check whether node meets layer requirement
******************/
bool myG::checkLayerDebug(P_SIZE p, bool bNodeType, int iPeelType, K_SIZE iMinK, K_SIZE iBigK, L_SIZE l, K_SIZE iEDegree)
{
    TPST_INDEX *pstRes = NULL;

    /*if (bNodeType)
    {
        DEBUG_PRINTF("CHECK u: %d (%d, %d) peel type: %d layer: %d\n", p, iMinK, iBigK, iPeelType, l);
    }
    else
    {
        DEBUG_PRINTF("CHECK v: %d (%d, %d) peel type: %d layer: %d\n", p, iMinK, iBigK, iPeelType, l);
    }*/

    // l-1 degree
    int iPrevDegree = 0;
    int iDegree = 0;

    for (P_SIZE pid : m_vAdj[bNodeType][p])
    {
        if (!m_vbA[!bNodeType][pid])
        {
            continue;
        }
        pstRes = findPoint(pid, !bNodeType, iPeelType, iMinK);
        if (pstRes != NULL)
        {
            //DEBUG_PRINTF("CHECK neighbor: %d (%d, %d) layer: %d\n", pid, iMinK, pstRes->k, pstRes->l);
            if (pstRes->k < iBigK)
            {
                // not in the (a, b)-core
                continue;
            }
            else if (pstRes->k > iBigK)
            {
                // in higher (a, b)-core
                ++iDegree;
                ++iPrevDegree;
                continue;
            }
            if (pstRes->l >= l)
            {
                ++iDegree;
            }
            if (l > 1)
            {
                if (pstRes->l >= l - 1)
                {
                    ++iPrevDegree;
                }
            }
        }
    }

    /*DEBUG_PRINTF("CHECK node: %d, type: %d peel: %d (%d, %d) layer: %d d: %d %d\n",
                 p, bNodeType, iPeelType, iMinK, iBigK, l, iPrevDegree, iDegree);*/
    if (iEDegree != iDegree)
    {
        DEBUG_PRINTF("CHECK node: %d, type: %d peel: %d (%d, %d) layer: %d d: %d %d estimate: %d\n",
                     p, bNodeType, iPeelType, iMinK, iBigK, l, iPrevDegree, iDegree, iEDegree);
        return false;
    }
    ASSERT_MSG(iEDegree == iDegree, "ERROR get degree: "<< iDegree <<" estimate: "<<iEDegree);
    if (iPeelType == 0)
    {
        K_SIZE a = iMinK;
        K_SIZE b = iBigK;
        // (a, b)-core -> (a, b+1)-core
        if (bNodeType)
        {
            //u
            DEBUG_ASSERT(l > 1);
            ASSERT_MSG(iPrevDegree >= a, "CHECK LAYER ERROR p: "<<p<<" type: "<<bNodeType);
            //DEBUG_ASSERT(iDegree < a);
            if (iDegree >= a)
            {
                //DEBUG_PRINTF("CHECK false 1\n");
                return false;
            }
        }
        else
        {
            // v
            if (l > 1)
            {
                DEBUG_ASSERT(iPrevDegree > b);
                //DEBUG_ASSERT(iDegree <= b);
                if (iDegree > b)
                {
                    //DEBUG_PRINTF("CHECK false 2\n");
                    return false;
                }
            }
            else
            {
                DEBUG_ASSERT(iDegree >= b);
                if (iDegree != b)
                {
                    //DEBUG_PRINTF("CHECK false 3\n");
                    return false;
                }
            }
        }
    }
    else if (iPeelType == 1)
    {
        K_SIZE a = iBigK;
        K_SIZE b = iMinK;
        // (a, b)-core -> (a+1, b)-core
        if (bNodeType)
        {
            // u
            if (l > 1)
            {
                ASSERT_MSG(iPrevDegree > a, "CHECK LAYER ERROR p: "<<p<<" type: "<<bNodeType);
                //DEBUG_ASSERT(iDegree <= a);
                if (iDegree > a)
                {
                    //DEBUG_PRINTF("CHECK false 4\n");
                    return false;
                }
            }
            else
            {
                DEBUG_ASSERT(iDegree >= a);
                if (iDegree != a)
                {
                    //DEBUG_PRINTF("CHECK false 5\n");
                    return false;
                }
            }
        }
        else
        {
            // v
            DEBUG_ASSERT(l > 1);
            DEBUG_ASSERT(iPrevDegree >= b);
            //DEBUG_ASSERT(iDegree < b);
            if (iDegree >= b)
            {
                //DEBUG_PRINTF("CHECK false 6\n");
                return false;
            }
        }
    }
    else
    {
        // (a, b)-core -> (a+1, b+1)-core
        DEBUG_ASSERT(iPeelType == 2);
        if (l > 1)
        {
            ASSERT_MSG(iPrevDegree > iMinK, "CHECK LAYER ERROR p: "<<p<<" type: "<<bNodeType);
            //DEBUG_ASSERT(iDegree <= a);
            if (iDegree > iMinK)
            {
                //DEBUG_PRINTF("CHECK false 7\n");
                return false;
            }
        }
        else
        {
            DEBUG_ASSERT(iDegree >= iMinK);
            if (iDegree != iMinK)
            {
                //DEBUG_PRINTF("CHECK false 8\n");
                return false;
            }
        }
    }

    return true;
}

/*****************
input:
        None
description:
        check whether all nodes meet layer requirement
******************/
bool myG::checkAll()
{
    bool res = true;
    // U
    DEBUG_ASSERT(m_vbA[1].size() > m_iMaxUId);
    DEBUG_ASSERT(m_vIndexA[1].size() > m_iMaxUId);
    DEBUG_ASSERT(m_vIndexB[1].size() > m_iMaxUId);
    DEBUG_ASSERT(m_vK[1].size() > m_iMaxUId);
    for (P_SIZE u = 1; u <= m_iMaxUId; ++u)
    {
        if (m_vbA[1][u])
        {
            for (int i = 0; i < m_vIndexA[1][u].size(); ++i)
            {
                int iMinK = i + 2;
                res = checkLayerDebug(u, true, 0, iMinK, m_vIndexA[1][u][i].k, m_vIndexA[1][u][i].l, m_vIndexA[1][u][i].d);
                ASSERT_MSG(res, "CHECK b FAIL u: " << u <<" (" << iMinK << ", " << m_vIndexA[1][u][i].k << ") l: "<<m_vIndexA[1][u][i].l<<endl);
            }
            for (int i = 0; i < m_vIndexB[1][u].size(); ++i)
            {
                int iMinK = i + 2;
                res = checkLayerDebug(u, true, 1, iMinK, m_vIndexB[1][u][i].k, m_vIndexB[1][u][i].l, m_vIndexB[1][u][i].d);
                ASSERT_MSG(res, "CHECK a FAIL u: " << u <<" (" << iMinK << ", " << m_vIndexB[1][u][i].k << ") l: "<<m_vIndexB[1][u][i].l<<endl);
            }
            int iMinK = m_vK[1][u].k;
            res = checkLayerDebug(u, true, 2, iMinK, iMinK, m_vK[1][u].l, m_vK[1][u].d);
            ASSERT_MSG(res, "CHECK k FAIL u: " << u <<" (" << iMinK << ", " << iMinK << ") l: "<<m_vK[1][u].l<<endl);

        }
    }
    // V
    DEBUG_ASSERT(m_vbA[0].size() > m_iMaxVId);
    DEBUG_ASSERT(m_vIndexA[0].size() > m_iMaxVId);
    DEBUG_ASSERT(m_vIndexB[0].size() > m_iMaxVId);
    DEBUG_ASSERT(m_vK[0].size() > m_iMaxVId);
    for (P_SIZE v = 1; v <= m_iMaxVId; ++v)
    {
        if (m_vbA[0][v])
        {
            for (int i = 0; i < m_vIndexA[0][v].size(); ++i)
            {
                int iMinK = i + 2;
                res = checkLayerDebug(v, false, 0, iMinK, m_vIndexA[0][v][i].k, m_vIndexA[0][v][i].l, m_vIndexA[0][v][i].d);
                ASSERT_MSG(res, "CHECK b FAIL v: " << v <<" (" << iMinK << ", " << m_vIndexA[0][v][i].k << ") l: "<<m_vIndexA[0][v][i].l<<endl);
            }
            for (int i = 0; i < m_vIndexB[0][v].size(); ++i)
            {
                int iMinK = i + 2;
                res = checkLayerDebug(v, false, 1, iMinK, m_vIndexB[0][v][i].k, m_vIndexB[0][v][i].l, m_vIndexB[0][v][i].d);
                ASSERT_MSG(res, "CHECK a FAIL v: " << v <<" (" << iMinK << ", " << m_vIndexB[0][v][i].k << ") l: "<<m_vIndexB[0][v][i].l<<endl);
            }
            int iMinK = m_vK[0][v].k;
            res = checkLayerDebug(v, false, 2, iMinK, iMinK, m_vK[0][v].l, m_vK[0][v].d);
            ASSERT_MSG(res, "CHECK k FAIL v: " << v <<" (" << iMinK << ", " << iMinK << ") l: "<<m_vK[0][v].l<<endl);
        }
    }
    return true;
}

/*****************
input:
        K_SIZE a
        K_SIZE b
        bool bPeelType
        vector<TPST_PROCESS> &vRaw
        vector<TPST_PROCESS> &vDone
description:
        edge process (layer adjust)
******************/
int myG::process(K_SIZE iMinK, K_SIZE iBigK, int iPeelType, vector<TPST_PROCESS> &vRaw, vector<TPST_PROCESS> &vDone)
{
    TPST_INDEX *pstRes = NULL;

    vSup.clear();
    vCurSup.clear();
    vTempDone.clear();
    vSameLayer.clear();

    m_vbRaw[1].resize(m_iMaxUId + 1);
    m_vbRaw[0].resize(m_iMaxVId + 1);
    m_vbSup[1].resize(m_iMaxUId + 1);
    m_vbSup[0].resize(m_iMaxVId + 1);
    m_vbDone[1].resize(m_iMaxUId + 1);
    m_vbDone[0].resize(m_iMaxVId + 1);
    m_vOldLayer[1].resize(m_iMaxUId + 1);
    m_vOldLayer[0].resize(m_iMaxVId + 1);

    //DEBUG_PRINTF("PROCESS (%d, %d) peel type: %d\n", iMinK, iBigK, iPeelType);

    for (auto atI : vRaw)
    {
        ASSERT_MSG(!(m_vbRaw[atI.bU][atI.p]), "ERROR already raw p: "<<atI.p<<", type: "<<atI.bU);
        m_vbRaw[atI.bU][atI.p] = true;
        /*if (atI.bU)
        {
            DEBUG_PRINTF("PROCESS raw u: %d l: %d\n", atI.p, atI.l);
        }
        else
        {
            DEBUG_PRINTF("PROCESS raw v: %d l: %d\n", atI.p, atI.l);
        }*/
    }
    L_SIZE iCurL = 0;
    while ((!vRaw.empty()) || (!vSup.empty()))
    {
        iCurL = L_MAX;
        // l from large to small
        if (!vRaw.empty())
        {
            sort(vRaw.begin(), vRaw.end(),
                [](const TPST_PROCESS& p1, const TPST_PROCESS& p2) {
                return (p1.l > p2.l);
                });
            iCurL = vRaw.back().l;
        }
        if (!vSup.empty())
        {
            // l from large to small
            sort(vSup.begin(), vSup.end(),
                [](const TPST_PROCESS& p1, const TPST_PROCESS& p2) {
                return (p1.l > p2.l);
                });
            iCurL = min(iCurL, (L_SIZE)(vSup.back().l + 1));
        }

        //DEBUG_PRINTF("PROCESS................... peel type: %d l: %d\n", iPeelType, iCurL);
        if (!vSup.empty())
        {
            // remove all nodes less than iCurL
            vCurSup.clear();
            while (!vSup.empty())
            {
                L_SIZE iSupL = vSup.back().l;
                if (iSupL >= iCurL)
                {
                    break;
                }
                vCurSup.push_back(vSup.back());
                vSup.pop_back();
            }
            for (auto atSup : vCurSup)
            {
                /*DEBUG_PRINTF("PROCESS remove support l: %d node: %d type: %d support: %d\n",
                             atSup.l, atSup.p, atSup.bU, m_vbSup[atSup.bU][atSup.p] == true);*/

                if (!m_vbSup[atSup.bU][atSup.p])
                {
                    // has been removed
                    continue;
                }
                pstRes = findPoint(atSup.p, atSup.bU, iPeelType, iMinK);
                //DEBUG_PRINTF("PROCESS layer: %d %d\n", stRes.r, stRes.u);
                DEBUG_ASSERT(pstRes);
                DEBUG_ASSERT(pstRes->k == iBigK);
                if (pstRes->l != atSup.l)
                {
                    // not this layer
                    continue;
                }
                /*if (atSup.bU)
                {
                    DEBUG_PRINTF("PROCESS support u: %d\n", atSup.p);
                }
                else
                {
                    DEBUG_PRINTF("PROCESS support v: %d\n", atSup.p);
                }*/
                for (P_SIZE iNeib : m_vAdj[atSup.bU][atSup.p])
                {
                    if (!m_vbA[!atSup.bU][iNeib])
                    {
                        // not exist
                        continue;
                    }
                    /*DEBUG_PRINTF("PROCESS neighbor %d type: %d raw: %d done: %d\n",
                                 iNeib, !atSup.bU,
                                 m_vbRaw[!atSup.bU][iNeib] == true, m_vbDone[!atSup.bU][iNeib] == true);*/
                    TPST_INDEX *pstN = findPoint(iNeib, !atSup.bU, iPeelType, iMinK);
                    if (!pstN)
                    {
                        // not exist
                        continue;
                    }
                    if (m_vbRaw[!atSup.bU][iNeib])
                    {
                        if (m_vOldLayer[atSup.bU][atSup.p])
                        {
                            // self upgraded
                            DEBUG_ASSERT(atSup.l < pstN->l);
                            pstN->d--;
                            L_SIZE iNeibL = pstN->l;
                            bool res = checkLayer(iNeib, !atSup.bU, iPeelType, iMinK);
                            // simply remove
                            if (res)
                            {
                                m_vbRaw[!atSup.bU][iNeib] = false;
                                //DEBUG_PRINTF("PROCESS pop %d type: %d from raw queue\n", iNeib, !atSup.bU);
                                // push into support layer
                                DEBUG_ASSERT(!m_vbSup[!atSup.bU][iNeib]);
                                m_vbSup[!atSup.bU][iNeib] = true;
                                vSup.push_back({!atSup.bU, iNeibL, iNeib});
                            }
                            continue;
                        }
                    }
                    if (m_vbDone[!atSup.bU][iNeib])
                    {
                        pstN->d--;
                        bool res = checkLayer(iNeib, !atSup.bU, iPeelType, iMinK);
                        if (res)
                        {
                            /*DEBUG_PRINTF("PROCESS pop from done queue and push into support queue, set layer: %d, node: %d type: %d\n",
                                         atSup.l + 1, iNeib, !atSup.bU);*/
                            //setLayer(iNeib, !atSup.bU, bPeelType, a, b, atSup.l + 1);
                            pstN->l = atSup.l + 1;

                            m_vbDone[!atSup.bU][iNeib] = false;
                            // push into support layer
                            DEBUG_ASSERT(!m_vbSup[!atSup.bU][iNeib]);
                            m_vbSup[!atSup.bU][iNeib] = true;
                            vSup.push_back({!atSup.bU, pstN->l, iNeib});
                        }
                        continue;
                    }
                    if (m_vbSup[!atSup.bU][iNeib])
                    {
                        L_SIZE iOldL = m_vOldLayer[!atSup.bU][iNeib];
                        if (0 < iOldL)
                        {
                            // neighbor upgraded
                            if ((atSup.l >= iOldL) && (atSup.l < pstN->l))
                            {
                                pstN->d--;
                            }
                        }
                        else
                        {
                            if (m_vOldLayer[atSup.bU][atSup.p])
                            {
                                // self upgraded
                                if (atSup.l < pstN->l)
                                {
                                    pstN->d--;
                                }
                            }
                        }
                    }
                }
                m_vbSup[atSup.bU][atSup.p] = false;
                m_vOldLayer[atSup.bU][atSup.p] = 0;
            }
        }

        // get same layer nodes
        DEBUG_ASSERT(vSameLayer.empty());
        while (!vRaw.empty())
        {
            if (vRaw.back().l == iCurL)
            {
                if (m_vbRaw[vRaw.back().bU][vRaw.back().p])
                {
                    // not removed
                    vSameLayer.push_back(vRaw.back());
                    m_vbRaw[vRaw.back().bU][vRaw.back().p] = false;
                }
                vRaw.pop_back();
            }
            else
            {
                break;
            }
        }
        // process nodes, increase layer to infinity
        for (auto atI : vSameLayer)
        {
            //DEBUG_PRINTF("PROCESS pop %d type: %d from raw queue, set max layer\n", atI.p, atI.bU);
            //setLayer(atI.p, atI.bU, bPeelType, a, b, L_MAX);
            pstRes = findPoint(atI.p, atI.bU, iPeelType, iMinK);
            DEBUG_ASSERT(pstRes);
            DEBUG_ASSERT(pstRes->k == iBigK);
            m_vOldLayer[atI.bU][atI.p] = pstRes->l;
            pstRes->l = L_MAX;

            ASSERT_MSG(!(m_vbDone[atI.bU][atI.p]), "ERROR already done p: "<<atI.p<<", type: "<<atI.bU);
            m_vbDone[atI.bU][atI.p] = true;
            //m_vbRaw[atI.bU][atI.p] = false;
            /*if (atI.bU)
            {
                DEBUG_PRINTF("PROCESS push done queue u: %d peel type: %d\n", atI.p, bPeelType);
            }
            else
            {
                DEBUG_PRINTF("PROCESS push done queue v: %d peel type: %d\n", atI.p, bPeelType);
            }*/
            vTempDone.push_back(atI);
        }
        // visit all neighbors,
        // in l layer and meet layer requirement: move to support queue
        // in l layer but not meet layer requirement: move to raw queue
        for (auto atI : vSameLayer)
        {
            P_SIZE iCurP = atI.p;
            bool bCurType = atI.bU;
            /*if (bCurType)
            {
                DEBUG_PRINTF("PROCESS current u: %d\n", atI.p);
            }
            else
            {
                DEBUG_PRINTF("PROCESS current v: %d\n", atI.p);
            }*/
            for (P_SIZE iNeib : m_vAdj[bCurType][iCurP])
            {
                if (!m_vbA[!bCurType][iNeib])
                {
                    // not exist
                    continue;
                }
                TPST_INDEX *pstN = findPoint(iNeib, !bCurType, iPeelType, iMinK);
                if ((!pstN) || (pstN->k != iBigK))
                {
                    // not in (a, b)-core
                    continue;
                }
                /*DEBUG_PRINTF("PROCESS find neighbor: %d k: %d layer: %d degree: %d\n",
                             iNeib, pstN->k, pstN->l, pstN->d);*/
                if (m_vbDone[!bCurType][iNeib])
                {
                    continue;
                }
                L_SIZE iDesL = pstN->l;
                if (m_vbRaw[!bCurType][iNeib])
                {
                    if (iDesL > iCurL)
                    {
                        pstN->d++;
                    }
                    continue;
                }
                //DEBUG_PRINTF("PROCESS neighbor: %d\n", iNeib);
                if (iDesL < iCurL)
                {
                    // cannot change
                    continue;
                }
                else if (iDesL > iCurL)
                {
                    pstN->d++;
                    bool res = checkLayer(iNeib, !bCurType, iPeelType, iMinK);
                    if (!res)
                    {
                        // push into raw queue
                        vRaw.push_back({!bCurType, iDesL, iNeib});
                        m_vbRaw[!bCurType][iNeib] = true;
                        //DEBUG_PRINTF("PROCESS added to raw queue: %d type: %d layer: %d\n", iNeib, !bCurType, iDesL);
                        if (m_vbSup[!bCurType][iNeib])
                        {
                            // label as being removed from support queue
                            m_vbSup[!bCurType][iNeib] = false;
                        }
                    }
                    else
                    {
                        // push into support queue
                        if (!m_vbSup[!bCurType][iNeib])
                        {
                            vSup.push_back({!bCurType, iDesL, iNeib});
                            m_vbSup[!bCurType][iNeib] = true;
                            //DEBUG_PRINTF("PROCESS 1 added to support queue: %d type: %d layer: %d\n", iNeib, !bCurType, iDesL);
                        }
                    }
                }
                else
                {
                    // push into support queue
                    if (!m_vbSup[!bCurType][iNeib])
                    {
                        vSup.push_back({!bCurType, iDesL, iNeib});
                        m_vbSup[!bCurType][iNeib] = true;
                        //DEBUG_PRINTF("PROCESS 2 added to support queue: %d type: %d layer: %d\n", iNeib, !bCurType, iDesL);
                    }
                }
            }
        }
        vSameLayer.clear();
    }

    if (iCurL > m_iMaxL)
    {
        m_iMaxL = iCurL;
    }
    for (auto atI : vTempDone)
    {
        if (m_vbDone[atI.bU][atI.p])
        {
            // in done queue
            vDone.push_back(atI);
            m_vbDone[atI.bU][atI.p] = false;
            m_vOldLayer[atI.bU][atI.p] = 0;
        }
    }
    //DEBUG_PRINTF("PROCESS temp done: %d real done: %d\n", vTempDone.size(), vDone.size());
    vTempDone.clear();
    DEBUG_ASSERT(vRaw.empty());
    DEBUG_ASSERT(vSup.empty());
    return 0;
}

/*****************
input:
        vector<TPST_CANDIDATE> &vCandidate
        int iPeelType
description:
        maintain one direction
******************/
int myG::maintain(int iPeelType)
{
	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;

	//m_vbNew[0].resize(m_iMaxVId+1);
	//m_vbNew[1].resize(m_iMaxUId+1);

    vector<TPST_CANDIDATE> *pvCandidate = &(vCandidate[iPeelType]);
    while (!(pvCandidate->empty()))
    {
        // first: b from large to small; second a from large to small
        // debug
        /*for (auto atI : vCandidate)
        {
            DEBUG_PRINTF("INSERT DEBUG before candidate: (%d %d) type: %d %d, l: %d node: %d\n",
                         atI.a, atI.b,
                         atI.bU, atI.bA,
                         atI.l, atI.p);
        }*/
        gettimeofday(&tv, NULL);
        lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;

        sort(pvCandidate->begin(), pvCandidate->end(),
            [](const TPST_CANDIDATE& p1, const TPST_CANDIDATE& p2) {
            if (p1.iMinK == p2.iMinK)
            {
                if (p1.iBigK == p2.iBigK)
                {
                    if (p1.l == p2.l)
                    {
                        if (p1.bU == p2.bU)
                        {
                            return (p1.p > p2.p);
                        }
                        else
                        {
                            return (p1.bU > p2.bU);
                        }
                    }
                    else
                    {
                        return (p1.l > p2.l);
                    }
                }
                else
                {
                    return (p1.iBigK > p2.iBigK);
                }
            }
            else
            {
                return (p1.iMinK > p2.iMinK);
            }
            });
        auto atI = unique(pvCandidate->begin(), pvCandidate->end(),
            [](const TPST_CANDIDATE& p1, const TPST_CANDIDATE& p2) {
            return (p1.iMinK == p2.iMinK) && (p1.iBigK == p2.iBigK) && (p1.bU == p2.bU) && (p1.l == p2.l) && (p1.p == p2.p);
            });
        pvCandidate->erase(atI, pvCandidate->end());
        gettimeofday(&tv, NULL);
        lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
        g_maintainSortTime += lCurTime - lStartTime;
        // debug
        /*for (auto atI : vCandidate)
        {
            DEBUG_PRINTF("INSERT DEBUG after candidate: (%d %d) type: %d %d, l: %d node: %d\n",
                         atI.a, atI.b,
                         atI.bU, atI.bA,
                         atI.l, atI.p);
        }*/

        TPST_CANDIDATE stCan = pvCandidate->back();

        /*DEBUG_PRINTF("INSERT current: (%d %d) type: %d %d, l: %d node: %lld\n",
                     stCan.a, stCan.b,
                     stCan.bU, stCan.bA,
                     stCan.l, stCan.p);*/
        K_SIZE iMinK = stCan.iMinK;
        K_SIZE iBigK = stCan.iBigK;
        //DEBUG_PRINTF("INSERT type: %d pair (%d %d)-----------------\n", iPeelType, iMinK, iBigK);
        DEBUG_ASSERT(vRaw.empty());
        DEBUG_ASSERT(vDone.empty());
        DEBUG_ASSERT(vCheck[0].empty());
        DEBUG_ASSERT(vCheck[1].empty());
        DEBUG_ASSERT(vCheck[2].empty());

        // find candidates with same a, b
        while (!pvCandidate->empty())
        {
            stCan = pvCandidate->back();
            if ((stCan.iMinK == iMinK) && (stCan.iBigK == iBigK))
            {
                TPST_INDEX *pstRes = findPoint(stCan.p, stCan.bU, iPeelType, iMinK);
                /*DEBUG_PRINTF("INSERT index: [%d, %d, (%d, %d)]\n",
                             m_vIndex[stCan.bU][stCan.p][iPos].a, m_vIndex[stCan.bU][stCan.p][iPos].b,
                             m_vIndex[stCan.bU][stCan.p][iPos].r, m_vIndex[stCan.bU][stCan.p][iPos].u);*/
                // skip changed nodes
                DEBUG_ASSERT(pstRes);
                if (pstRes->k == iBigK)
                {
                    // not consider layer changed node
                    if (pstRes->l == stCan.l)
                    {
                        //DEBUG_PRINTF("INSERT raw: %d type: %d l: %d\n", stCan.p, stCan.bU, stCan.l);
                        vRaw.push_back({stCan.bU, stCan.l, stCan.p});
                    }
                }

                /*DEBUG_PRINTF("INSERT pop candidate: (%d %d) type: %d %d, l: %d node: %d\n",
                             stCan.a, stCan.b,
                             stCan.bU, stCan.bA,
                             stCan.l, stCan.p);*/
                pvCandidate->pop_back();
            }
            else
            {
                break;
            }
        }
        /*DEBUG_PRINTF("INSERT raw size: %d %d\n",
                     vRaw[0].size(), vRaw[1].size());*/

        //DEBUG_PRINTF("INSERT k pair (%d %d) peel: %d\n", iMinK, iBigK, iPeelType);

        if (!vRaw.empty())
        {
            gettimeofday(&tv, NULL);
            lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
            process(iMinK, iBigK, iPeelType, vRaw, vDone);
            gettimeofday(&tv, NULL);
            lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
            g_maintainProcessTime += lCurTime - lStartTime;

            if (!vDone.empty())
            {
                g_maintainUpgradeCnt += vDone.size();
                // increase max k
                if (0 == iPeelType)
                {
                    // b direction
                    if (iBigK >= m_iMaxB)
                    {
                        m_iMaxB = iBigK + 1;
                    }
                }
                else if (1 == iPeelType)
                {
                    // a direction
                    if (iBigK >= m_iMaxA)
                    {
                        m_iMaxA = iBigK + 1;
                    }
                }
                else
                {
                    if (iBigK >= m_iMaxK)
                    {
                        m_iMaxK = iBigK + 1;
                        m_iMaxA = max(m_iMaxA, m_iMaxK);
                        m_iMaxB = max(m_iMaxB, m_iMaxK);
                    }
                }
            }

            gettimeofday(&tv, NULL);
            lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
            for (auto atI : vDone)
            {
                //DEBUG_PRINTF("INSERT done: %d type: %d\n", atI.p, atI.bU);
                L_SIZE iBaseLA = 1;
                L_SIZE iBaseLB = 1;
                if (atI.bU)
                {
                    iBaseLB = 2;
                }
                else
                {
                    iBaseLA = 2;
                }
                // for correctly updating degree
                //m_vbNew[atI.bU][atI.p] = true;
                 if (0 == iPeelType)
                {
                    // b direction
                    TPST_INDEX *pstRes = findPoint(atI.p, atI.bU, iPeelType, iMinK);
                    pstRes->k = iBigK + 1;
                    pstRes->l = iBaseLB;
                    newIndex(atI.p, atI.bU, iPeelType, iMinK, iBigK + 1, iBaseLB);
                    //vCheck[0].push_back({iMinK, iBigK + 1, atI.bU, iBaseLB, atI.p});
                }
                else if (1 == iPeelType)
                {
                    // a direction
                    TPST_INDEX *pstRes = findPoint(atI.p, atI.bU, iPeelType, iMinK);
                    pstRes->k = iBigK + 1;
                    pstRes->l = iBaseLA;
                    newIndex(atI.p, atI.bU, iPeelType, iMinK, iBigK + 1, iBaseLA);
                    //vCheck[1].push_back({iMinK, iBigK + 1, atI.bU, iBaseLA, atI.p});
                }
                else
                {
                    DEBUG_ASSERT(2 == iPeelType)
                    K_SIZE iKMax = m_vK[atI.bU][atI.p].k;
                    DEBUG_ASSERT(iMinK == iKMax);
                    // increase k_max(p)
                    ++iKMax;
                    m_vK[atI.bU][atI.p].k = iKMax;
                    m_vK[atI.bU][atI.p].l = 1;
                    newIndex(atI.p, atI.bU, 2, iKMax, iKMax, 1);
                    // new a or b index
                    m_vIndexA[atI.bU][atI.p].push_back({iKMax, iKMax, iBaseLB});
                    newIndex(atI.p, atI.bU, 0, iKMax, iKMax, iBaseLB);
                    m_vIndexB[atI.bU][atI.p].push_back({iKMax, iKMax, iBaseLA});
                    newIndex(atI.p, atI.bU, 1, iKMax, iKMax, iBaseLA);
                    //m_vbNew[atI.bU][atI.p] = true;
                    ASSERT_MSG(m_vIndexA[atI.bU][atI.p].size() == iKMax - 1, "ERROR size: "<<m_vIndexA[atI.bU][atI.p].size()<<" k: "<<iKMax);
                    DEBUG_ASSERT(m_vIndexB[atI.bU][atI.p].size() == iKMax - 1);
                }
            }

            for (auto atI : vDone)
            {
                //DEBUG_PRINTF("INSERT check: %d type: %d\n", atI.p, atI.bU);
                L_SIZE iBaseLA = 1;
                L_SIZE iBaseLB = 1;
                if (atI.bU)
                {
                    iBaseLB = 2;
                }
                else
                {
                    iBaseLA = 2;
                }
                //m_vbNew[atI.bU][atI.p] = false;
                 if (0 == iPeelType)
                {
                    // b direction
                    checkIndexAndNeib(atI.p, atI.bU, iPeelType, iMinK);
                }
                else if (1 == iPeelType)
                {
                    // a direction
                    checkIndexAndNeib(atI.p, atI.bU, iPeelType, iMinK);
                }
                else
                {
                    K_SIZE iKMax = m_vK[atI.bU][atI.p].k;
                    checkIndexAndNeib(atI.p, atI.bU, 2, iKMax);
                    // new a or b index
                    estimateAndCheck(atI.p, atI.bU, 0, iKMax);
                    estimateAndCheck(atI.p, atI.bU, 1, iKMax);
                }
            }
            /*if (2 == iPeelType)
            {
                for (auto atI : vDone)
                {
                    m_vbNew[atI.bU][atI.p] = false;
                }
            }*/
            gettimeofday(&tv, NULL);
            lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
            g_maintainDoneTime += lCurTime - lStartTime;

            vDone.clear();
            DEBUG_ASSERT(vRaw.empty());
        }
        //DEBUG_PRINTF("INSERT one batch done candidate size: %d\n", pvCandidate->size());
    }
    return 0;
}
/*****************
input:
        P_SIZE x
        P_SIZE y
description:
        edge insertion
******************/
bool myG::simpleInsert(P_SIZE x, P_SIZE y)
{
    //DEBUG_PRINTF("INSERT edge (%d %d)==============\n", x, y);

    if (x > m_iMaxUId)
    {
        m_iMaxUId = x;
        m_vAdj[1].resize(m_iMaxUId + 1);
        m_vD[1].resize(m_iMaxUId + 1);
        m_vbA[1].resize(m_iMaxUId + 1);
        m_vIndexA[1].resize(m_iMaxUId + 1);
        m_vIndexB[1].resize(m_iMaxUId + 1);
        m_vK[1].resize(m_iMaxUId + 1);
    }
    if (y > m_iMaxVId)
    {
        m_iMaxVId = y;
        m_vAdj[0].resize(m_iMaxVId + 1);
        m_vD[0].resize(m_iMaxVId + 1);
        m_vbA[0].resize(m_iMaxVId + 1);
        m_vIndexA[0].resize(m_iMaxVId + 1);
        m_vIndexB[0].resize(m_iMaxVId + 1);
        m_vK[0].resize(m_iMaxVId + 1);
    }
    if (!m_vbA[1][x])
    {
        m_vbA[1][x] = true;
        // init
        //m_vMax[1][x] = pair<K_SIZE, K_SIZE>(1, 1);
        m_vK[1][x] = (TPST_INDEX){1, 0, 1};
    }
    if (!m_vbA[0][y])
    {
        m_vbA[0][y] = true;
        //m_vMax[0][y] = pair<K_SIZE, K_SIZE>(1, 1);
        m_vK[0][y] = (TPST_INDEX){1, 0, 1};
    }

    P_SIZE iOldSize = m_vD[1][x];
    //print(true, x);

    adjInsert(m_vAdj[1][x], y);
    m_vD[1][x] = m_vAdj[1][x].size();
    if (m_vD[1][x] > m_iUMaxD)
    {
        m_iUMaxD =m_vD[1][x];
    }
    if (iOldSize == m_vD[1][x])
    {
        // no new edge
        DEBUG_PRINTF("INSERT edge (%d %d) exist, same degree: %d\n", x, y, iOldSize);
        //print(true, x);
        return false;
    }
    ++m_iMaxEId;
    adjInsert(m_vAdj[0][y], x);
    m_vD[0][y] = m_vAdj[0][y].size();
    if (m_vD[0][y] > m_iVMaxD)
    {
        m_iVMaxD = m_vD[0][y];
    }
    return true;
}

/*****************
input:
        P_SIZE x
        P_SIZE y
description:
        edge insertion
******************/
K_SIZE myG::estimateK(P_SIZE p, bool bU, int iPeelType, K_SIZE iBaseK)
{
    K_SIZE k = 0;
    K_SIZE iMaxK = 0;
    vCnt.clear();
    if (iPeelType)
    {
        vCnt.resize(m_iMaxA + 1);
    }
    else
    {
        vCnt.resize(m_iMaxB + 1);
    }
    for (P_SIZE iNeib : m_vAdj[bU][p])
    {
        TPST_INDEX *pstN = findPoint(iNeib, !bU, iPeelType, iBaseK);
        if (!pstN)
        {
            continue;
        }
        ASSERT_MSG(pstN->k < vCnt.size(), "ERROR get k: "<<pstN->k<<" max: "<<vCnt.size());
        vCnt[pstN->k]++;
        iMaxK = max(iMaxK, pstN->k);
    }
    K_SIZE iCnt = 0;
    if (bU == iPeelType || 2 == iPeelType)
    {
        // bU == iPeelType or 2 == iPeelType
        k = iMaxK;
        for (; k > 0; --k)
        {
            iCnt += vCnt[k];
            //DEBUG_PRINTF("ESTIMATE_K size: %d k: %d current: %d minD: %d cnt: %d\n", vCnt.size(), k, vCnt[k], k, iCnt);
            if (iCnt >= k)
            {
                // in the k-core
                break;
            }
        }
    }
    else
    {
        // bU != iPeelType and 2 != iPeelType
        k = iMaxK;
        for (; k > 0; --k)
        {
            iCnt += vCnt[k];
            // bU != iPeelType
            //DEBUG_PRINTF("ESTIMATE_K k: %d current: %d minD: %d cnt: %d\n", k, vCnt[k], iBaseK, iCnt);
            if (iCnt >= iBaseK)
            {
                // in the k-core
                break;
            }
        }
    }

    DEBUG_ASSERT(k > 0);
    return k;
}
/*****************
input:
        P_SIZE x
        P_SIZE y
description:
        edge insertion
******************/
int myG::newIndex(P_SIZE p, bool bU, int iPeelType, K_SIZE iBaseK, K_SIZE iBigK, L_SIZE l)
{
    // check and push into candidate queue
    TPST_INDEX *pstRes = findPoint(p, bU, iPeelType, iBaseK);
    ASSERT_MSG(pstRes, "ERROR k: "<<iBaseK);
    pstRes->k = iBigK;
    pstRes->l = l;
    K_SIZE iDegree = 0;

    //DEBUG_PRINTF("CHECK_INDEX check neighbors start\n");
    for (P_SIZE iNeib : m_vAdj[bU][p])
    {
        if (!m_vbA[!bU][iNeib])
        {
            // not exist
            continue;
        }
        /*if (m_vbNew[!bU][iNeib])
        {
            ++iDegree;
            continue;
        }*/
        //DEBUG_PRINTF("PROCESS neighbor: %d\n", iNeib);
        TPST_INDEX *pstNeib = findPoint(iNeib, !bU, iPeelType, iBaseK);
        /*DEBUG_PRINTF("PROCESS find neighbor: %d pos: %d point: [%d, %d, (%d, %d)]\n",
                     iNeib, iPos, stRes.a, stRes.b, stRes.r, stRes.u);*/
        if ((!pstNeib) || (pstNeib->k < iBigK))
        {
            // not in (a, b)-core
            continue;
        }
        if (pstNeib->k > iBigK)
        {
            ++iDegree;
            continue;
        }
        L_SIZE iDesL = pstNeib->l;
        if (iDesL > l)
        {
            // cannot change
            ++iDegree;
            continue;
        }
        if (iDesL == l)
        {
            // same layer
            ++iDegree;
        }
        pstNeib->d++;
        //DEBUG_PRINTF("NEW_INDEX increase neighbor: %d type: %d degree: %d\n", iNeib, !bU, pstNeib->d);
    }
    pstRes->d = iDegree;
    //DEBUG_PRINTF("NEW_INDEX (%d, %d) peel: %d layer: %d node: %d type: %d degree: %d\n", iBaseK, iBigK, iPeelType, l, p, bU, iDegree);
    return 0;
}
/*****************
input:
        P_SIZE x
        P_SIZE y
description:
        edge insertion
******************/
int myG::checkIndexAndNeib(P_SIZE p, bool bU, int iPeelType, K_SIZE iBaseK)
{
	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;
    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;

    // check and push into candidate queue
    TPST_INDEX *pstRes = findPoint(p, bU, iPeelType, iBaseK);
    ASSERT_MSG(pstRes, "ERROR k: "<<iBaseK);

    //DEBUG_PRINTF("CHECK_INDEX check node: %d type: %d peel: %d ((%d, %d), %d).............\n", p, bU, iPeelType, iBaseK, pstRes->k, pstRes->l);
    bool bRes = checkLayer(p, bU, iPeelType, iBaseK);
    if (!bRes)
    {
        /*DEBUG_PRINTF("MAINTAIN peel: %d push %d type: %d (%d %d) l: %d\n",
                     iType, atI.p, atI.bU, atI.iMinK, atI.iBigK, atI.l);*/
        vCandidate[iPeelType].push_back({iBaseK, pstRes->k, bU, pstRes->l, p});
    }
    //DEBUG_PRINTF("CHECK_INDEX check neighbors start size: %d\n", m_vAdj[bU][p].size());
    for (P_SIZE iNeib : m_vAdj[bU][p])
    {
        if (!m_vbA[!bU][iNeib])
        {
            // not exist
            continue;
        }
        //DEBUG_PRINTF("CHECK_INDEX neighbor: %d\n", iNeib);
        TPST_INDEX *pstN = findPoint(iNeib, !bU, iPeelType, iBaseK);
        if ((!pstN) || (pstN->k != pstRes->k))
        {
            // not in (a, b)-core
            continue;
        }
        /*DEBUG_PRINTF("CHECK_INDEX find neighbor: %d point: [%d, %d, (%d, %d)]\n",
                     iNeib, iBaseK, pstN->k, pstN->l, pstN->d);*/
        L_SIZE iDesL = pstN->l;
        if (iDesL > pstRes->l)
        {
            // cannot change
            continue;
        }
        //DEBUG_PRINTF("CHECK_INDEX check neighbor begin\n");
        bRes = checkLayer(iNeib, !bU, iPeelType, iBaseK);
        //DEBUG_PRINTF("CHECK_INDEX check neighbor done: %d\n", bRes);
        if (!bRes)
        {
            /*DEBUG_PRINTF("MAINTAIN peel: %d push %d type: %d (%d %d) l: %d\n",
                         iType, iNeib, !atI.bU, iBaseK, atI.iBigK, iDesL);*/
            vCandidate[iPeelType].push_back({iBaseK, pstRes->k, !bU, iDesL, iNeib});
        }
    }
    //DEBUG_PRINTF("CHECK_INDEX check neighbors done\n");
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
    g_checkIndexTime += lCurTime - lStartTime;
    return 0;
}
/*****************
input:
        P_SIZE x
        P_SIZE y
description:
        new index, check self and neighbors
******************/
int myG::estimateAndCheck(P_SIZE p, bool bU, int iPeelType, K_SIZE iBaseK)
{
	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;
    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;

    TPST_INDEX *pstNode = findPoint(p, bU, iPeelType, iBaseK);
    K_SIZE iOldK = pstNode->k;
    L_SIZE iOldL = pstNode->l;
    /*DEBUG_PRINTF("ESTIMATE start node: %d, type: %d peel %d: base: %d long: %d layer: %d\n",
                 p, bU, iPeelType, iBaseK, iOldK, iOldL);*/
    K_SIZE iNewK = estimateK(p, bU, iPeelType, iBaseK);
    ASSERT_MSG(iNewK >= iOldK, "ERROR k: "<<iOldK<<" "<<iNewK);
    K_SIZE iNewBase = iBaseK;
    L_SIZE iNewL = 1;
    K_SIZE iDegree = 0;
    if (1 == iPeelType)
    {
        if (!bU)
        {
            iNewL = 2;
        }
    }
    else if (0 == iPeelType)
    {
        if (bU)
        {
            iNewL = 2;
        }
    }
    if (iOldK == iNewK)
    {
        iNewL = max(iNewL, iOldL);
    }
    // change index
    pstNode->k = iNewK;
    pstNode->l = iNewL;
    /*DEBUG_PRINTF("ESTIMATE node: %d, type: %d peel %d: base: %d long: %d %d layer: %d %d\n",
                 p, bU, iPeelType, iBaseK, iOldK, iNewK, iOldL, iNewL);*/
    if (2 == iPeelType)
    {
        DEBUG_ASSERT(0);
        /*iNewBase = iNewK;
        if (iNewK > iOldK)
        {
            for (K_SIZE k = iOldK; k < iNewK; ++k)
            {
                m_vIndexA[bU][p].push_back({k+1, 1 + (L_SIZE)bU});
                checkIndex(p, bU, 0, k+1, k+1, 1 + (L_SIZE)bU);
                m_vIndexB[bU][p].push_back({k+1, 1 + (L_SIZE)!bU});
                checkIndex(p, bU, 1, k+1, k+1, 1 + (L_SIZE)!bU);
            }
        }*/
    }
    // index changed, update neighbors' degree
    for (P_SIZE iNeib : m_vAdj[bU][p])
    {
        TPST_INDEX *pstN = findPoint(iNeib, !bU, iPeelType, iBaseK);
        if (!pstN)
        {
            continue;
        }
        //DEBUG_PRINTF("ESTIMATE %d neighbor: %d type: %d (%d %d) l: %d degree: %d\n", iPeelType, iNeib, !bU, iBaseK, pstN->k, pstN->l, pstN->d);
        if (pstN->k < iOldK)
        {
            continue;
        }
        /*else if (pstN->k == iOldK)
        {
            if (pstN->l < iOldL)
            {
                continue;
            }
        }*/
        if (pstN->k > iNewK)
        {
            ++iDegree;
            continue;
        }
        else if (pstN->k == iNewK)
        {
            //DEBUG_PRINTF("ESTIMATE layer: %d %d\n", pstN->l, iNewL);
            if (pstN->l > iNewL)
            {
                ++iDegree;
                continue;
            }
            else if (pstN->l == iNewL)
            {
                ++iDegree;
            }
        }
        // new index, check later
        /*if (m_vbNew[!bU][iNeib])
        {
            continue;
        }*/
        if ((pstN->k > iOldK) || ((pstN->k == iOldK) && (pstN->l > iOldL)))
        {
            if ((pstN->k < iNewK) || ((pstN->k == iNewK) && (pstN->l <= iNewL)))
            {
                pstN->d++;
            }
        }
        //DEBUG_PRINTF("ESTIMATE %d neighbor: %d type: %d (%d %d) l: %d degree: %d\n", iPeelType, iNeib, !bU, iBaseK, pstN->k, pstN->l, pstN->d);
        bool bRes = checkLayer(iNeib, !bU, iPeelType, iBaseK);
        if (!bRes)
        {
            //DEBUG_PRINTF("ESTIMATE %d neighbor push: %d type: %d (%d %d) l: %d\n", iPeelType, iNeib, !bU, iBaseK, pstN->k, pstN->l);
            vCandidate[iPeelType].push_back({iBaseK, pstN->k, !bU, pstN->l, iNeib});
        }
    }

    pstNode->d = iDegree;
    // check self
    //DEBUG_PRINTF("ESTIMATE check node: %d type: %d peel: %d ((%d, %d), %d, %d).............\n", p, bU, iPeelType, iBaseK, iNewK, iNewL, iDegree);
    bool bRes = checkLayer(p, bU, iPeelType, iBaseK);
    if (!bRes)
    {
        //DEBUG_PRINTF("ESTIMATE %d push u: %d (%d %d) l: %d\n", iPeelType, p, iBaseK, iNewK, iNewL);
        vCandidate[iPeelType].push_back({iBaseK, iNewK, bU, iNewL, p});
    }
    // new index, must check neighbors
    // no change, no need to check neighbors
    /*if ((iOldK == iNewK) && (iOldL == iNewL))
    {
        return 0;
    }*/
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
    g_estimateTime += lCurTime - lStartTime;
    return 0;
}
/*****************
input:
        P_SIZE x
        P_SIZE y
description:
        edge insertion
******************/
int myG::findSeed(P_SIZE x, P_SIZE y)
{
    DEBUG_ASSERT(x <= m_iMaxUId);
    DEBUG_ASSERT(y <= m_iMaxVId);
    int iMinK = min(m_vK[1][x].k, m_vK[0][y].k);

    int iRes = 0;
    bool bRes = false;

    // Step 1: find seed, low order node may be upgraded
    // k direction
    TPST_INDEX *pstMin = minLayer(x, true, y, false, 2, iMinK, &iRes);
    DEBUG_ASSERT(pstMin);
    if (iRes <= 0)
    {
        TPST_INDEX *pstX = findPoint(x, true, 2, iMinK);
        pstX->d++;
        bRes = checkLayer(x, true, 2, iMinK);
        if (!bRes)
        {
            //DEBUG_PRINTF("INSERT k push u: %d (%d %d) l: %d\n", x, iMinK, iMinK, pstMin->l);
            vCandidate[2].push_back({iMinK, iMinK, true, pstMin->l, x});
        }
    }
    if (iRes >= 0)
    {
        TPST_INDEX *pstY = findPoint(y, false, 2, iMinK);
        pstY->d++;
        bRes = checkLayer(y, false, 2, iMinK);
        if (!bRes)
        {
            //DEBUG_PRINTF("INSERT k push v: %d (%d %d) l: %d\n", y, iMinK, iMinK, pstMin->l);
            vCandidate[2].push_back({iMinK, iMinK, false, pstMin->l, y});
        }
    }
    // a and b directions
    for (int iPeelType : {0, 1})
    {
        for (int iCurK = 2; iCurK <= iMinK; ++iCurK)
        {
            TPST_INDEX *pstMin = minLayer(x, true, y, false, iPeelType, iCurK, &iRes);
            DEBUG_ASSERT(pstMin);
            if (iRes <= 0)
            {
                TPST_INDEX *pstX = findPoint(x, true, iPeelType, iCurK);
                pstX->d++;
                bRes = checkLayer(x, true, iPeelType, iCurK);
                if (!bRes)
                {
                    //DEBUG_PRINTF("INSERT peel: %d push u: %d (%d %d) l: %d\n", iPeelType, x, iCurK, pstMin->k, pstMin->l);
                    vCandidate[iPeelType].push_back({iCurK, pstMin->k, true, pstMin->l, x});
                }
            }
            if (iRes >= 0)
            {
                TPST_INDEX *pstY = findPoint(y, false, iPeelType, iCurK);
                pstY->d++;
                bRes = checkLayer(y, false, iPeelType, iCurK);
                if (!bRes)
                {
                    //DEBUG_PRINTF("INSERT peel: %d push v: %d (%d %d) l: %d\n", iPeelType, y, iCurK, pstMin->k, pstMin->l);
                    vCandidate[iPeelType].push_back({iCurK, pstMin->k, false, pstMin->l, y});
                }
            }
        }
    }

    return 0;
}
/*****************
input:
        P_SIZE x
        P_SIZE y
description:
        edge insertion
******************/
int myG::edge_insert(P_SIZE x, P_SIZE y)
{
    bool bRes = simpleInsert(x, y);
    if (!bRes)
    {
        // exists
        return 0;
    }

    // Step 1: find seed, low order node may be upgraded
    findSeed(x, y);

    // Step 2: maintenance
    // first maintain k direction
    //DEBUG_PRINTF("INSERT maintain k\n");
    maintain(2);
    //DEBUG_PRINTF("INSERT maintain b\n");
    maintain(0);
    //DEBUG_PRINTF("INSERT maintain a\n");
    maintain(1);

    DEBUG_ASSERT(vCandidate[2].empty());
    DEBUG_ASSERT(vCandidate[0].empty());
    DEBUG_ASSERT(vCandidate[1].empty());

    // debug
    //DEBUG_PRINTF("INSERT check all\n");
    //checkAll();
    return 0;
}

/*****************
input:
        batch of edges and output vector
description:
        apply edge-disjoint color classes in parallel
******************/
int myG::batchInsertEdgesParallel(
    const vector<pair<P_SIZE, P_SIZE> > &vQuery,
    vector<pair<P_SIZE, P_SIZE> > &vRealQ)
{
    if (vQuery.empty())
    {
        return 0;
    }

    P_SIZE maxU = m_iMaxUId;
    P_SIZE maxV = m_iMaxVId;
    for (const auto &edge : vQuery)
    {
        maxU = max(maxU, edge.first);
        maxV = max(maxV, edge.second);
    }

    m_iMaxUId = maxU;
    m_iMaxVId = maxV;
    m_vAdj[1].resize(maxU + 1);
    m_vD[1].resize(maxU + 1);
    m_vbA[1].resize(maxU + 1);
    m_vIndexA[1].resize(maxU + 1);
    m_vIndexB[1].resize(maxU + 1);
    m_vK[1].resize(maxU + 1);
    m_vAdj[0].resize(maxV + 1);
    m_vD[0].resize(maxV + 1);
    m_vbA[0].resize(maxV + 1);
    m_vIndexA[0].resize(maxV + 1);
    m_vIndexB[0].resize(maxV + 1);
    m_vK[0].resize(maxV + 1);

    set<P_SIZE> seenU;
    set<P_SIZE> seenV;
    for (const auto &edge : vQuery)
    {
        const P_SIZE x = edge.first;
        const P_SIZE y = edge.second;
        if (seenU.insert(x).second && !m_vbA[1][x])
        {
            m_vbA[1][x] = true;
            m_vK[1][x] = (TPST_INDEX){1, 0, 1};
        }
        if (seenV.insert(y).second && !m_vbA[0][y])
        {
            m_vbA[0][y] = true;
            m_vK[0][y] = (TPST_INDEX){1, 0, 1};
        }
    }

    vector<unsigned char> inserted(vQuery.size(), 0);
    map<P_SIZE, vector<size_t> > groupedU;
    map<P_SIZE, vector<size_t> > groupedV;
    for (size_t edgeId = 0; edgeId < vQuery.size(); ++edgeId)
    {
        groupedU[vQuery[edgeId].first].push_back(edgeId);
        groupedV[vQuery[edgeId].second].push_back(edgeId);
    }

    vector<pair<P_SIZE, vector<size_t> > > uGroups;
    vector<pair<P_SIZE, vector<size_t> > > vGroups;
    for (auto &group : groupedU)
    {
        uGroups.push_back(make_pair(group.first, move(group.second)));
    }
    for (auto &group : groupedV)
    {
        vGroups.push_back(make_pair(group.first, move(group.second)));
    }

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 32) if(uGroups.size() >= 128)
#endif
    for (P_SIZE groupId = 0; groupId < static_cast<P_SIZE>(uGroups.size()); ++groupId)
    {
        const P_SIZE x = uGroups[groupId].first;
        vector<P_SIZE> additions;
        for (size_t edgeId : uGroups[groupId].second)
        {
            const P_SIZE y = vQuery[edgeId].second;
            if (!binary_search(m_vAdj[1][x].begin(), m_vAdj[1][x].end(), y))
            {
                inserted[edgeId] = 1;
                additions.push_back(y);
            }
        }
        if (!additions.empty())
        {
            sort(additions.begin(), additions.end());
            vector<P_SIZE> merged;
            merged.reserve(m_vAdj[1][x].size() + additions.size());
            merge(m_vAdj[1][x].begin(), m_vAdj[1][x].end(),
                  additions.begin(), additions.end(), back_inserter(merged));
            m_vAdj[1][x].swap(merged);
            m_vD[1][x] = m_vAdj[1][x].size();
        }
    }

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 32) if(vGroups.size() >= 128)
#endif
    for (P_SIZE groupId = 0; groupId < static_cast<P_SIZE>(vGroups.size()); ++groupId)
    {
        const P_SIZE y = vGroups[groupId].first;
        vector<P_SIZE> additions;
        for (size_t edgeId : vGroups[groupId].second)
        {
            if (inserted[edgeId])
            {
                additions.push_back(vQuery[edgeId].first);
            }
        }
        if (!additions.empty())
        {
            sort(additions.begin(), additions.end());
            vector<P_SIZE> merged;
            merged.reserve(m_vAdj[0][y].size() + additions.size());
            merge(m_vAdj[0][y].begin(), m_vAdj[0][y].end(),
                  additions.begin(), additions.end(), back_inserter(merged));
            m_vAdj[0][y].swap(merged);
            m_vD[0][y] = m_vAdj[0][y].size();
        }
    }

    for (size_t edgeId = 0; edgeId < vQuery.size(); ++edgeId)
    {
        if (!inserted[edgeId])
        {
            continue;
        }
        const P_SIZE x = vQuery[edgeId].first;
        const P_SIZE y = vQuery[edgeId].second;
        ++m_iMaxEId;
        m_iUMaxD = max(m_iUMaxD, m_vD[1][x]);
        m_iVMaxD = max(m_iVMaxD, m_vD[0][y]);
        vRealQ.push_back(vQuery[edgeId]);
    }
    return 0;
}

/*****************
input:
        P_SIZE x
        P_SIZE y
description:
        edge insertion
******************/
int myG::batch_insert(vector<pair<P_SIZE, P_SIZE> > &vQuery)
{
    return batchInsertImpl(vQuery, false);
}

int myG::batch_insert_parallel(vector<pair<P_SIZE, P_SIZE> > &vQuery)
{
    return batchInsertImpl(vQuery, true);
}

int myG::batchInsertImpl(vector<pair<P_SIZE, P_SIZE> > &vQuery, bool parallel)
{
	struct timeval tv;
	long lStartTime = 0;
	long lCurTime = 0;

    vector<pair<P_SIZE, P_SIZE> > vRealQ;
    vRealQ.reserve(vQuery.size());

    // Step 1: find seed, low order node may be upgraded
    //DEBUG_PRINTF("INSERT simple insert start============\n");
    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
    if (parallel)
    {
        vector<pair<P_SIZE, P_SIZE> > uniqueQuery;
        uniqueQuery.reserve(vQuery.size());
        set<pair<P_SIZE, P_SIZE> > seenEdges;
        for (const auto &edge : vQuery)
        {
            if (seenEdges.insert(edge).second)
            {
                uniqueQuery.push_back(edge);
            }
        }
        batchInsertEdgesParallel(uniqueQuery, vRealQ);
    }
    else for (auto atIt : vQuery)
    {
        P_SIZE x = atIt.first;
        P_SIZE y = atIt.second;

        bool bRes = simpleInsert(x, y);
        if (!bRes)
        {
            // exists
            continue;
        }
        //DEBUG_PRINTF("INSERT (%d, %d)\n", x, y);
        vRealQ.push_back(atIt);
    }
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
    g_simpleTime += lCurTime - lStartTime;

    if (vRealQ.empty())
    {
        return 0;
    }

    //DEBUG_PRINTF("INSERT estimate start============\n");
    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
    for (auto atIt : vRealQ)
    {
        P_SIZE x = atIt.first;
        P_SIZE y = atIt.second;
        findSeed(x, y);
    }
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
    g_findTime += lCurTime - lStartTime;

    // Step 2: maintenance
    // first maintain k direction
    //DEBUG_PRINTF("INSERT maintain k, size: %d============\n", vCandidate[2].size());
    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
    maintain(2);
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
    g_maintainTime2 += lCurTime - lStartTime;
    //DEBUG_PRINTF("INSERT maintain b, size: %d============\n", vCandidate[0].size());
    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
    maintain(0);
    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
    g_maintainTime0 += lCurTime - lStartTime;
    //DEBUG_PRINTF("INSERT maintain a, size: %d============\n", vCandidate[1].size());
    gettimeofday(&tv, NULL);
    lStartTime = tv.tv_sec * 1000000 + tv.tv_usec;
    maintain(1);

    gettimeofday(&tv, NULL);
    lCurTime = tv.tv_sec * 1000000 + tv.tv_usec;
    g_maintainTime1 += lCurTime - lStartTime;

    DEBUG_ASSERT(vCandidate[2].empty());
    DEBUG_ASSERT(vCandidate[0].empty());
    DEBUG_ASSERT(vCandidate[1].empty());

    // debug
    //DEBUG_PRINTF("INSERT check all\n");
    //checkAll();
    return 0;
}


