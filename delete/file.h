#pragma once
/***************
input file output file function
****************/

#define ONE_LINE_BUFFER 100

// index structure
// 100 chars:   U_max V_max alpha_max beta_max m indexSize
// U blocks:    [(d, k, l), ...]
// V blocks:    [(d, k, l), ...]
// U a index blocks:    [(a, l), ...]
// U b index blocks:    [(a, l), ...]
// V a index blocks:    [(a, l), ...]
// V b index blocks:    [(a, l), ...]
// edges: [v, ...]

typedef struct save_node
{
    //P_SIZE n;
    P_SIZE d;
    K_SIZE k;
    K_SIZE ld;
    L_SIZE l;
} SaveNode;

typedef struct save_index
{
    K_SIZE k;
    K_SIZE d;
    L_SIZE l;
} SaveIndex;

//int file_readG(char *pcFile, myG &oMpG);
int file_query(char *pcFile, vector<pair<P_SIZE, P_SIZE> > &vQuery);
int fileBatchSave(int fd, char *pcData, long long llTotoalSize);
int fileBatchRead(int fd, char *pcData, long long llTotoalSize);

int file_saveBitIndex(myG &oMpG, char *pcIndexFile);
int file_readBitIndex(myG &oMpG, char *pcIndexFile);
