CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./src
LDFLAGS = -lopengl32 -lgdi32 -luser32 -lcomdlg32 -lgdiplus -lcomctl32 -mwindows

TARGET = build/PixelForge.exe
SRCS = src/main.cpp src/core/application.cpp src/ui/main_window.cpp

all: directories $(TARGET)

directories:
	@mkdir -p build
	@if exist src\resources mkdir -p build\resources && xcopy /E /Y src\resources build\resources > nul

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf build

.PHONY: all clean directories 