CXX = g++
CXXFLAGS = -DWIN32 -D_DEBUG -D_WINDOWS -D_MBCS -DMINGW
ALL_SOURCES = *.cpp
OBJECTS = brainBayRes.o $(patsubst %.cpp,%.o,$(wildcard *.cpp))
  

Debug/brainBay.exe: $(OBJECTS)
	g++ -o Debug/brainBay -mwindows -LDebug --enable-auto-import -L. -Llibmingw *.o -lSDL -lSDL_sound -lSDL_net \
	-lopengl32 -lglu32 -lwinmm -lcomctl32 -lvfw_avi32 -lvfw_ms32 -lfidlib \
	-lcv -lcvcam -lcxcore -lhighgui -lole32 -lolepro32 -luuid

brainBayRes.o: brainBay.rc
	windres -i brainBay.rc -o brainBayRes.o

depends:
	$(CXX) $(CXXFLAGS) -MM $(ALL_SOURCES) > depends

-include depends

clean:
	rm *.o
	rm Debug/brainBay.exe
