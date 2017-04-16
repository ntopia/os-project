CXX = arm-linux-gnueabi-gcc
CXXFLAGS = -O2

all: rot_test selector trial

rot_test: rot_test.c
	$(CXX) -o $@ $^ $(CXXFLAGS)

selector: selector.c
	$(CXX) -o $@ $^ $(CXXFLAGS)

trial: trial.c
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean:
	rm -f *.o rot_test selector trial
