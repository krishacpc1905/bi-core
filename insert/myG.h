#pragma once
/***************
my class G
****************/

/*typedef struct tpstAdj
{
    P_SIZE iPid;
    E_SIZE iEid;
}TPST_ADJ;*/

typedef struct tpstLayer
{
    P_SIZE p;
    K_SIZE k;
    K_SIZE d;
    L_SIZE l;
}TPST_LAYER;

typedef struct tpstIndex
{
    K_SIZE k;
    K_SIZE d;
    L_SIZE l;
}TPST_INDEX;

typedef struct tpstCandidate
{
    K_SIZE iMinK;
    K_SIZE iBigK;
    bool bU;    // true: U; false: V
    L_SIZE l;
    P_SIZE p;
}TPST_CANDIDATE;

typedef struct tpstProcess
{
    bool bU;    // true: U; false: V
    L_SIZE l;
    P_SIZE p;
}TPST_PROCESS;

class myG
{
private:
    //static int save(vector<vector<TPST_INDEX> > &vIndex, vector<TPST_LAYER> &vLayer, K_SIZE iFixK, bool bFirst);
    //static int refine(vector<vector<TPST_INDEX> > &vIndex, vector<pair<K_SIZE, K_SIZE> > &vMax);
    static int adjInsert(vector<P_SIZE> &vAdj, P_SIZE p);
    bool simpleInsert(P_SIZE x, P_SIZE y);
    int batchInsertEdgesParallel(
        const vector<pair<P_SIZE, P_SIZE> > &vQuery,
        vector<pair<P_SIZE, P_SIZE> > &vRealQ);
    int batchInsertImpl(vector<pair<P_SIZE, P_SIZE> > &vQuery, bool parallel);
    int findSeed(P_SIZE x, P_SIZE y);

    TPST_INDEX *findPoint(P_SIZE p, bool bNodeType, int iPeelType, K_SIZE k);

    bool checkLayer(P_SIZE p, bool bNodeType, int iPeelType, K_SIZE iMinK);
    bool checkLayerDebug(P_SIZE p, bool bNodeType, int iPeelType, K_SIZE iMinK, K_SIZE iBigK, L_SIZE l, K_SIZE iDegree);
    TPST_INDEX * minLayer(P_SIZE x, bool bXType, P_SIZE y, bool bYType, int iPeelType, K_SIZE k, int *pRes);
    //bool setLayer(P_SIZE p, bool bNodeType, bool bPeelType, K_SIZE a, K_SIZE b, L_SIZE l);
    //P_SIZE countDegree(P_SIZE p, bool bNodeType, bool bPeelType, K_SIZE a, K_SIZE b, L_SIZE l);

    // iPeelType: 0 b; 1 a; 2 both
    int process(K_SIZE iMinK, K_SIZE iBigK, int iPeelType, vector<TPST_PROCESS> &vRaw, vector<TPST_PROCESS> &vDone);
    int maintain(int iPeelType);

    K_SIZE estimateK(P_SIZE p, bool bU, int iPeelType, K_SIZE iBaseK);
    //L_SIZE estimateL(P_SIZE p, bool bU, int iPeelType, K_SIZE iMinK, K_SIZE iBigK);
    int estimateAndCheck(P_SIZE p, bool bU, int iPeelType, K_SIZE iBaseK);

    int newIndex(P_SIZE p, bool bU, int iPeelType, K_SIZE iBaseK, K_SIZE iBigK, L_SIZE l);
    int checkIndexAndNeib(P_SIZE p, bool bU, int iPeelType, K_SIZE iBaseK);

    // debug
    bool checkAll();

    // process
    vector<bool> m_vbRaw[2];
    vector<bool> m_vbSup[2];
    vector<bool> m_vbDone[2];
    vector<L_SIZE> m_vOldLayer[2];

    vector<TPST_PROCESS> vSup;
    vector<TPST_PROCESS> vCurSup;
    vector<TPST_PROCESS> vTempDone;
    vector<TPST_PROCESS> vSameLayer;

    // maintain
    vector<TPST_PROCESS> vRaw;
    vector<TPST_PROCESS> vDone;
    vector<TPST_CANDIDATE> vCheck[3];
    //vector<bool> m_vbNew[2];

    // insertion
    vector<TPST_CANDIDATE> vCandidate[3];

    // estimate k: <k, count>
    vector<int> vCnt;

public:
    // 1: u; 0: v
    vector<vector<P_SIZE> > m_vAdj[2];
    vector<P_SIZE> m_vD[2];

    // node available
    vector<bool> m_vbA[2];

    // index
    vector<vector<TPST_INDEX> > m_vIndexA[2]; // fix a peeling b
    vector<vector<TPST_INDEX> > m_vIndexB[2]; // fix b peeling a
    vector<TPST_INDEX> m_vK[2];

    // alpha_max, beta_max
    //vector<pair<K_SIZE, K_SIZE> > m_vMax[2];

    P_SIZE m_iMaxUId;
    P_SIZE m_iMaxVId;
    E_SIZE m_iMaxEId;
    P_SIZE m_iUMaxD;
    P_SIZE m_iVMaxD;
    K_SIZE m_iMaxA;
    K_SIZE m_iMaxB;
    K_SIZE m_iMaxK;
    L_SIZE m_iMaxL;

    myG();
    ~myG();
    //int init();

    //int index();
    int edge_insert(P_SIZE x, P_SIZE y);
    int batch_insert(vector<pair<P_SIZE, P_SIZE> > &vQuery);
    int batch_insert_parallel(vector<pair<P_SIZE, P_SIZE> > &vQuery);

    //static int peeling(vector<vector<P_SIZE> > &vAdjA, vector<vector<P_SIZE> > &vAdjB, vector<P_SIZE> &m_vDA, vector<P_SIZE> &m_vDB, vector<bool> &m_vbA, vector<bool> &m_vbB, K_SIZE iKA, vector<TPST_LAYER> &vAL, vector<TPST_LAYER> &vBL);

    // debug
    int print(bool bType, P_SIZE p);
    int print();
};
