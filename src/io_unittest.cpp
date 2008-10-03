#include <gtest/gtest.h>

#include "io.h"

//goals
// 1. Simplicity.
// 2. Speed.
// 3. Ease of reading/writing binary formats.

//everything is read into a buffer.
void write(Io & in){
  int a;
  Io failure("");
  failure.GetInt(&a); EXPECT_EQ(true, failure.error());

  EXPECT_EQ( 0, in.size()); //what's inside
  EXPECT_EQ( 0, in.get_position());
  in.PutInt(10).PutFloat(20.0f);
  EXPECT_EQ( 8, in.get_position());
  in.rewind(); EXPECT_EQ( 0, in.get_position());
};

void read(Io & in){
  float f;
  int i;
  EXPECT_EQ(8, in.size()); //what's inside
  EXPECT_EQ(0, in.get_position());
  in.GetInt(&i).GetFloat(&f);
  EXPECT_EQ(10, i);
  EXPECT_EQ(20.0f, f);
  EXPECT_EQ(8, in.get_position());
  in.rewind(); EXPECT_EQ(0, in.get_position());
};

TEST(Io, Serialization){
  Io in("");
  write(in);
  read(in);
};

TEST(Io, File){
  std::string in = ReadFile("lh.pial");
  std::string none = ReadFile("lh_unexistent.pial");
  int string_size = in.size();
  EXPECT_EQ(3940566, string_size);
  EXPECT_EQ(0, none.size());
  system("rm lh_test.pial");
  EXPECT_TRUE(WriteFile("lh_test.pial", in));
  std::string test = ReadFile("lh_test.pial");
  EXPECT_EQ(string_size, test.size());
};
