#pragma once

#include <vector>
#include <string>
#include <chrono>

using namespace std;

#include "../src/image/inc/image.h"
#include "../src/utils/utils.h"

int tests_total;
int tests_fail;
#define TEST(EX) do { ++tests_total; if(!(EX)) {\
    fprintf(stderr, "failed: [%s] testing [%s] in %s, line %d\n", __FUNCTION__, #EX, __FILE__, __LINE__); \
    ++tests_fail; }} while (0)
