#include <gtest/gtest.h>
#include "surface.h"

#include <iostream>
#include <string>

class SurfaceTest: public testing::Test {
public:
  Surface surf;
  Surface a;
  Surface b;
};

TEST_F(SurfaceTest, Header) {
  int triangles_number, vertices_number;
  Io data(ReadFile("rh.pial"));
  EXPECT_TRUE(ReadPialHeader(data, &vertices_number, &triangles_number));
  printf("%d:%d vertices:triangles numbers\n", 
	 vertices_number,
	 triangles_number);
};

/// 1 BSQ
TEST_F(SurfaceTest, IO) {
  EXPECT_EQ(surf.v.size()+surf.n.size()+surf.c.size()+surf.tri.size()+surf.seeds.size(), 0);
  EXPECT_TRUE(read_surface_binary(surf, "rh.pial"));
  EXPECT_TRUE(read_surface_binary(a, "rh.pial"));
  EXPECT_TRUE(read_surface_binary(b, "lh.pial"));
  EXPECT_TRUE(surf.v.size() > 0);
  EXPECT_TRUE(b.v.size() > 0);
  EXPECT_EQ(b.v.size(), b.n.size());
  EXPECT_EQ(surf.v.size(), surf.n.size());
  EXPECT_EQ(surf.seeds.size(), 1);
  //Ensure all normals are of unit length.
  int cnt = 0;
  for(vector<V3f>::iterator i = surf.n.begin(); i != surf.n.end(); i++){
    ASSERT_FLOAT_EQ(i->length(), 1.0) << "Vertex number " << cnt << "\n";
    cnt++;
  };

  //Load next chunk.
  EXPECT_TRUE(read_surface_binary(surf, "lh.pial"));
  EXPECT_EQ(surf.seeds.size(), 2); //Now 2 seeds are avaliable. Checking them is too involved.
  EXPECT_EQ(surf.v.size(), a.v.size() + b.v.size());
  EXPECT_EQ(surf.n.size(), a.n.size() + b.n.size());
  //Ensure all normals are still of unit length.

  cnt = 0;

  for(vector<V3f>::iterator i = surf.n.begin(); i != surf.n.end(); i++){
    EXPECT_FLOAT_EQ(i->length(), 1.0) << "Vertex number " << cnt << "\n";
    cnt++;
  };



  clear(surf);
  EXPECT_EQ(surf.v.size()+surf.n.size()+surf.c.size()+surf.tri.size()+surf.seeds.size(), 0);
};



