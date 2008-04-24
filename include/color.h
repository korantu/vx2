#ifndef __COLOR_H__
#define __COLOR_H__

/*
  This file defines structures for color mapping;
*/

struct ColorMapper{
  ColorMapper(); //fills in default colors
  unsigned char col[256*3];

  void map(void *, int); 
  void map(int & r, int & g, int &b, int); 
};

void scheme_fill(ColorMapper &, int scheme);

#endif
