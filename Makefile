CXX = g++
CXXFLAGS = -std=c++17 -Wall
LDFLAGS = -lopengl32 -lgdi32 -luser32

TARGET = PixelForge.exe
SRCS = src/main.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	del $(TARGET)

.PHONY: all clean 