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

// when a >= b, k is a; when a <= b, k is b
typedef struct tpstIndex
{
    K_SIZE k;
    K_SIZE d;
    L_SIZE l;
}TPST_INDEX;

class myG
{
private:
    int save(vector<vector<TPST_INDEX> > &vIndex, vector<TPST_LAYER> &vLayer);
    int save(bool bU, vector<TPST_LAYER> &vLayer);
    //int refine(vector<vector<TPST_INDEX> > &vIndex, vector<pair<K_SIZE, K_SIZE> > &vMax);

    // peeling
    vector<P_SIZE> vARmQ;
    vector<P_SIZE> vBRmQ;
    vector<P_SIZE> vARmQBack;
    vector<P_SIZE> vBRmQBack;
    vector<vector<P_SIZE> > vBin;

    vector<P_SIZE> vNextA;
    vector<P_SIZE> vNextB;

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
    //vector<pair<K_SIZE, K_SIZE> > m_vUMax;
    //vector<pair<K_SIZE, K_SIZE> > m_vVMax;

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
    int init();

    int index();

    /* will not copy map, will copy array */
    int adjInsert(vector<vector<P_SIZE> > &vAdj, P_SIZE x, P_SIZE y);

    int peelingUnit(vector<vector<P_SIZE> > &vAdjA, vector<vector<P_SIZE> > &vAdjB, vector<P_SIZE> &vRemainA, vector<P_SIZE> &vRemainB, vector<P_SIZE> &vDA, vector<P_SIZE> &vDB, K_SIZE iDesKA, K_SIZE iDesKB, vector<TPST_LAYER> &vAL, vector<TPST_LAYER> &vBL);
    int peeling(vector<vector<P_SIZE> > &vAdjA, vector<vector<P_SIZE> > &vAdjB, vector<P_SIZE> &vRemainA, vector<P_SIZE> &vRemainB, vector<P_SIZE> &m_vDA, vector<P_SIZE> &m_vDB, K_SIZE iKA, vector<TPST_LAYER> &vAL, vector<TPST_LAYER> &vBL);

    // debug
    int print(bool bType, P_SIZE p);
    int print();
};
