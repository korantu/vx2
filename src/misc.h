#ifndef __misc_h__
#define __misc_h__

//TODO
#define MAX2(a,b) (((a)>(b))?(a):(b))
#define ABS(a) (MAX2((a),(-(a))))
#define MAX3(a,b,c) (MAX2(MAX2(a,b),c))
#define CHOOSE_MAX(a, b, c, da, db, dc) (((a > b) && (a > c))?(da):((b>c)?(db):(dc)))

float smooth_bell(float x);

/// Determines if the state of the object is valid.
/// Rationale - check if a sequence of operations succeeded.

class Validatable {
 public:
  Validatable();

  virtual bool valid();
  virtual void valid(bool is_valid);

  virtual operator bool();

 private:
  bool valid_;
};

#endif __misc_h__

