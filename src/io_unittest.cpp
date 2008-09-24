#include <gtest/gtest.h>

#include "io.h"

//goals
// 1. Simplicity.
// 2. Speed.
// 3. Ease of reading/writing binary formats.

//everything is read into a buffer.
void create_unexisting(){
  Io in("unexistent");
  EXPECT_EQ(in.size(), 0); //what's inside
  EXPECT_EQ(in.pos(), 0);
  in.putInt(10).putFloat(20.0f);
  EXPECT_EQ(in.pos(), 8);
  in.rewind(); EXPECT_EQ(in.pos(), 0);
};

void verify_unexisting(){
  Io in("unexistent");
  EXPECT_EQ(in.size(), 0); //what's inside
  EXPECT_EQ(in.pos(), 0);
  EXPECT_EQ(in.getInt(), 100);
  EXPECT_EQ(in.getFloat(), 1000);
  EXPECT_EQ(in.pos(), 8);
  in.rewind(); EXPECT_EQ(in.pos(), 0);
};

TEST(Io, Loading){
  create_unexisting();
  verify_unexisting();
};
