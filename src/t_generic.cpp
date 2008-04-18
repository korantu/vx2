#include "unit--.h"
#include "io.h"

testSuite(MySuite);

testCase(CompareCase, MySuite)
{
    int x = 1;
    int y = x + 2;
    assertTrue(x < y);
}

