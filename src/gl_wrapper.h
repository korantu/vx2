#ifndef __GL_WRAPPER__
#define __GL_WRAPPER__


///key definition:


/// an event; comes down;
struct gl_wrapper_reciever{

  ///state
  struct {
    int width;  // window params
    int height;
    int x; //current mouse pos
    int y;
    int w; //whell
    int dx; //change of values
    int dy;
    int dw;
    int k;    //currently pressed key, if any
    int m_b;    //currently pressed m/b, if any
    bool interface_updated;
  } st;


//structure for recieving an event.
  virtual void do_mouse()=0;
  virtual void do_key()=0;
  virtual void do_wheel()=0;
  virtual void do_resize()=0;
  virtual void do_render()=0;


  gl_wrapper_reciever(); ///`initialize the whole structure.
  virtual ~gl_wrapper_reciever();
};

void gl_init(gl_wrapper_reciever *);
void gl_stop();

#endif // __GL_WRAPPER__
