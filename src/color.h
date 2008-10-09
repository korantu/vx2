#ifndef __COLOR_H__
#define __COLOR_H__

/*
  This file defines structures for color mapping;
*/

#include "v3.h"
#include <vector>
#include <string>

struct ColorMapper{
  ColorMapper(); //fills in default colors
  ColorMapper(const ColorMapper &); //fills in default colors
  unsigned char col[256*3];

  ColorMapper & operator=(const ColorMapper &);

  void map(void *, int); 
  void map(int); //use opengl Color3i 
  void map(int & r, int & g, int &b, int); 

  void fill_init(V3f _f);
  void fill(int end, V3f to);
  void fill(const int *);

  int start;
  V3f from;

};

void color_init();
std::vector<std::string> color_type();

void scheme_fill(ColorMapper &, int scheme);

#endif
