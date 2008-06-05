#ifndef __GUI_H__
#define __GUI_H__

#include "slices.h"
#include "gl_points.h"

void gui_start(slices *, GlPoints *);
void gui_draw();
void gui_resize(int w, int h);
void gui_stop();

void set_current_file(std::string in);

void read_voxels(std::string in, GlPoints * pnt, bool _half, bool _tru, bool _fill);


#endif // __GUI_H__
