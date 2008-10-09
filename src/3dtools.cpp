#include "3dtools.h"

//Implementation of various 3d visualization tools.

// Crossection display.

Crossection & Crossection::Resize(int, int){
  return *this;
};

Crossection & Crossection::Position(V3f &){
  return *this;
};

Crossection & Crossection::Orientation(V3f &){
  return *this;
};

Crossection & Crossection::Draw(){
  return *this;
}; 
