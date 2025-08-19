CXX = g++
CXXFLAGS = -Wall -O2 $(shell pkg-config --cflags opencv4)
LDFLAGS = $(shell pkg-config --libs opencv4) -lblake3

SRC = main.cpp FileTree.cpp FileInfo.cpp Utility.cpp Checksum.cpp BKTree.cpp Manager.cpp
OBJ = $(SRC:.cpp=.o)

TARGET = output

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
