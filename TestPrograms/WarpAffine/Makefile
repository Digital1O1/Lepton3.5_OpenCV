CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall

OPENCV_CFLAGS := $(shell pkg-config --cflags opencv4)
OPENCV_LIBS   := $(shell pkg-config --libs opencv4)
OTHER_LIBS := -lncurses

TARGET := warpAffineExample
SRC := main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(OPENCV_CFLAGS) $^ -o $@ $(OPENCV_LIBS) $(OTHER_LIBS)

clean:
	rm -f $(TARGET)

