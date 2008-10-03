#ifndef __io_h__
#define __io_h__

#include <string>

// Copying possible; content is not shared; so prefere reference

class Io {
 public:
  //top-level
  Io(std::string);
  ~Io();

  //technical
  int size();
  int get_position();
  int set_position();
  Io & rewind();
  bool error(); // Return true if this stream is valid;

  //main
  Io & GetInt(int * result);
  Io & GetFloat(float * res);
  Io & GetShort(short * res);
  Io & GetChar(char * res);
  
  Io & PutInt(int res);
  Io & PutFloat(float res);
  Io & PutShort(short res);
  Io & PutChar(char res);
 
 private:
  std::string content_;
  int position_;
  bool error_;
  void CopyRawData(const char * source, int amount, char * destination);
  bool EnsureBytesAvaliable(int amount);

  template<class T>
    friend Io & Write(Io & in, T *);

  template<class T>
    friend Io & Read(Io & in, T *);

};
 
//File io.
std::string ReadFile(std::string name);
bool WriteFile( std::string name, std::string contents);

#endif __io_h__
