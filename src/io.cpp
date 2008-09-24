#include <io.h>

using namespace std;

Io::Io(string){
};

Io::~Io(){
};

  //technical
int Io::size(){return 0;};
int Io::pos(){return 0;};
Io & Io::rewind(){return *this;};
  
  //reading
int Io::getInt(){return 1;};
int Io::getFloat(){return 1.0f;};
  
  //writing
Io & Io::putInt(int){return *this;};
Io & Io::putFloat(float){return *this;};
  
