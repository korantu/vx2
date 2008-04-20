CXXFLAGS := -DUSE_TW -g -O2 -I../../include -I../../deps/anttweakbar/include -I../../deps/glfw/include -I../../deps/glfw/include/GL 
LINKFLAGS := ../../deps/glfw/lib/x11/libglfw.a ../../deps/anttweakbar/lib/libant.a -lX11 -lXrandr -lGL -lGLU -lz   
VX2 := navigator.o v3tools.o fastvolume.o loader.o gl_points.o gl_wrapper.o vx2.o 
VX2_SRC :=  ../../src/v3tools.cpp \
	../../src/fastvolume.cpp \
	../../src/loader.cpp \
	../../src/gl_points.cpp \
	../../src/gl_wrapper.cpp \
	../../src/vx2.cpp

deps: $(VX2_SRC)
	$(CXX) $(CXXFLAGS) -M $(VX2_SRC) > deps.txt

vx2: $(VX2)
	$(CXX) $(VX2) $(LINKFLAGS) -o vx2


.PHONY: clean
clean:
	rm *.o  

#objects
%.o: ../../src/%.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@  

-include deps.txt





