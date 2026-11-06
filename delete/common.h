#pragma once

using namespace std;

#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <list>
#include <map>
#include <utility>
#include <algorithm>
#include <iterator>
#include <vector>
#include <queue>
#include <memory.h>
#include <unordered_map>
#include <set>

#define DEBUG_PRINT_BTN
#define DEBUG_ASSERT_BTN
#include "debug.h"
#include "graph.h"

template<typename T>
void COMMON_UNIQUE(vector<T> &vDes)
{
    sort(vDes.begin(), vDes.end());
    vDes.erase(unique(vDes.begin(), vDes.end()), vDes.end());
}
