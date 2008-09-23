#include <gtest/gtest.h>
#include "surface.h"

#include <iostream>
#include <string>

class SurfaceTest: public testing::Test {
public:
  Surface surf;
};

TEST_F(SurfaceTest, IO) {
  EXPECT_TRUE(read_surface_binary(surf, "rh.pial"));
  std::cout << surf.v.size();
};



