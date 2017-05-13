CXX = arm-linux-gnueabi-gcc
GCCFLAGS = -O2


all: wrrtool trial

trial: trial.c test.h
	$(CXX) -o $@ $^ $(CXXFLAGS)

wrrtool: wrrtool.c test.h
	$(CXX) -o $@ $^ $(CXXFLAGS)

temp: temp.c
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -f temp wrrtool trial *.o


