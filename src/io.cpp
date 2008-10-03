#include "io.h"

using namespace std;

Io::Io(string content): content_(content), position_(0), error_(false) {};
Io::~Io(){};

  //technical
int Io::size(){return content_.size();};
Io & Io::rewind(){position_=0; return *this;};
int Io::get_position(){return position_;};
bool Io::error(){return error_;};

  //Parametrized function to read different types using IO.
template<class T> 
Io & Read(Io & in, T * result){
  if ((in.content_.size() - in.position_)>=sizeof(*result)) {
    for (int i = 0; i < sizeof(*result); i++) {
      ((char *)result)[i] = in.content_[in.position_ + sizeof(*result) - i - 1];
    };

    in.position_ += sizeof(*result); // Updating position.
      } else {
    in.error_ = true;
  };
  return in;
};

//Parametrized function to write different types using IO.
template<class T> 
Io & Write(Io & in, T * result){
  string to_write((char *) result, sizeof(*result));
  
  for (int i = 0; i < sizeof(*result); i++){ //Copy bytes.
    to_write[sizeof(*result) - i - 1] = ((char *)result)[i];
  };
  
  if ((in.size() - in.get_position()) > sizeof(*result)) { //Replace.
    in.content_.replace(in.get_position(), sizeof(*result), to_write);
  } else if (in.get_position() == in.size()) { //Append.
    in.content_ += to_write;
  } else { //Too little space to replace and not appending; Error.
    in.error_ = true;
  };

  in.position_ += sizeof(*result); // Updating position.
  return  in;   
}; 

  //reading
Io & Io::GetInt(int * result){ return Read<int>(*this, result); };
Io & Io::GetFloat(float * result){return Read<float>(*this, result); };
Io & Io::GetShort(short * result){return Read<short>(*this, result); };
Io & Io::GetChar(char * result){return Read<char>(*this, result); };
  
  //writing
Io & Io::PutInt(int result){return Write<int>(*this, &result); };
Io & Io::PutFloat(float result){return Write<float>(*this, &result); };
Io & Io::PutShort(short result){return Write<short>(*this, &result); };
Io & Io::PutChar(char result){return Write<char>(*this, &result); };

//File io.

string ReadFile(string name){
  FILE* file = fopen(name.c_str(), "rb");
  if (file == NULL) return "";

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char* chars = new char[size];

  for (int i = 0; i < size;) {
    int read = fread(&chars[i], 1, size - i, file);
    i += read;
  }
  fclose(file);
  string result(chars, size);
  delete[] chars;
  
  return result;
};

//Write file
bool WriteFile( std::string name, std::string contents){
  FILE* file = fopen(name.c_str(), "wb");
  if (file == NULL) return false;
  int bytes_written = fwrite(contents.c_str(), 1, contents.size(), file);
  fclose(file);

  return bytes_written == contents.size();
};
