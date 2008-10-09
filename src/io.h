#ifndef __io_h__
#define __io_h__

#include <string>

#include "misc.h"

// Copying possible; content is not shared; so prefere reference

class Io: public Validatable {
 public:
  //top-level
  Io(std::string);
  ~Io();

  //Technical book keeping.
  int size();
  int get_position();
  Io & set_position(int position);
  std::string content();

  Io & rewind();

  //Chained versions.
  Io & GetInt(int * result);
  Io & GetFloat(float * res);
  Io & GetShort(short * res);
  Io & GetChar(char * res);
  
  //Return versions.
  int GetInt();
  float GetFloat();
  short GetShort();
  char GetChar();

  Io & PutInt(int res);
  Io & PutFloat(float res);
  Io & PutShort(short res);
  Io & PutChar(char res);
 
  Io & ReplaceContent(std::string new_contnet);

 private:
  std::string content_;
  int position_;
  void CopyRawData(const char * source, int amount, char * destination);
  bool EnsureBytesAvaliable(int amount);

  template<class T>
    friend Io & Write(Io & in, T *);

  template<class T>
    friend Io & Read(Io & in, T *);

};
 
//File io. Empty strings are returned if reading fails.

std::string ReadFile(std::string name);
std::string ReadGzipFile(std::string name);
bool WriteFile( std::string name, std::string contents);
bool WriteGzipFile( std::string name, std::string contents);

#endif __io_h__
