#include <string>

// Class IO will use common buffer set; so that the copies could be freely passed around.
// Concurrency beware;

class Io {
  //top-level
  Io(std::string);
  ~Io();

  //technical
  int size();
  int pos();
  Io & rewind();
  
  //reading
  int getInt();
  int getFloat();
  
  //writing
  Io & putInt(int);
  Io & putFloat(float);
  
};
