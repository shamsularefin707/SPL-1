# =============================================================================
#  FaceMatrix - Module 1 (Image Processing) build file
#
#  make          build the demo and the unit tests
#  make run      build + run the demo     (writes demo_*.bmp in this folder)
#  make test     build + run the unit tests
#  make clean    delete everything the build created
#
#  Linux / macOS / WSL / Git Bash / MSYS2 : use `make`
#  Windows cmd or PowerShell with MinGW  : use `mingw32-make` (clean: `mingw32-make clean-win`)
# =============================================================================

# ---- Compiler and flags ------------------------------------------------------
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
# -Isrc lets the code write #include "image/Image.h"
CPPFLAGS += -Isrc
# -MMD -MP: write a .d file listing the headers each .cpp uses
DEPFLAGS  = -MMD -MP

# ---- .exe suffix on Windows --------------------------------------------------
ifeq ($(OS),Windows_NT)
  EXE := .exe
else
  EXE :=
endif

# ---- Folders and files -------------------------------------------------------
SRC_DIR   = src/image
BUILD_DIR = build

IMAGE_SRCS = $(SRC_DIR)/Image.cpp \
             $(SRC_DIR)/ImageLoader.cpp \
             $(SRC_DIR)/Grayscale.cpp \
             $(SRC_DIR)/Resize.cpp \
             $(SRC_DIR)/Contrast.cpp \
             $(SRC_DIR)/Histogram.cpp \
             $(SRC_DIR)/Normalization.cpp \
             $(SRC_DIR)/Rotate.cpp \
             $(SRC_DIR)/TextRender.cpp

# src/image/Image.cpp  ->  build/Image.o   (one object file per source file)
IMAGE_OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(IMAGE_SRCS))

DEMO = fer_image_demo$(EXE)
TEST = fer_image_tests$(EXE)

# ---- Targets that are commands, not files ------------------------------------
.PHONY: all run test clean clean-win

all: $(DEMO) $(TEST)

# Create the build folder once (order-only prerequisite, see "| $(BUILD_DIR)")
$(BUILD_DIR):
	mkdir $(BUILD_DIR)

# Pattern rule: compile any src/image/X.cpp into build/X.o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

# Link the demo program
$(DEMO): src/main.cpp $(IMAGE_OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

# Link the unit-test program
$(TEST): tests/test_image_processing.cpp $(IMAGE_OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ -o $@

run: $(DEMO)
	./$(DEMO)

test: $(TEST)
	./$(TEST)

clean:
	rm -rf $(BUILD_DIR) $(DEMO) $(TEST) fer_image_demo.exe fer_image_tests.exe \
	       sample_input.bmp demo_*.bmp temp_test_bmp

clean-win:
	-if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	-del /q $(DEMO) $(TEST) sample_input.bmp demo_*.bmp 2>NUL
	-if exist temp_test_bmp rmdir /s /q temp_test_bmp

# Pull in the generated header dependencies (silently skipped on the first build)
-include $(IMAGE_OBJS:.o=.d)
