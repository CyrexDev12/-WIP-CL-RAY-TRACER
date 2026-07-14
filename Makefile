# ==============================================================================
# Compiler settings
# ==============================================================================
CXX      := g++
# Added -Isrc so paths resolve directly from the src directory root
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -MMD -MP -Isrc

# ==============================================================================
# Directory layout
# ==============================================================================
SRC_DIR   := src
BUILD_DIR := build
TARGET    := raytracer.exe

# ==============================================================================
# File tracking
# ==============================================================================
# Explicit module groups mirror the CMake target boundaries. Keep this build path
# available during migration; CMake is the canonical build system.
SRCS := $(wildcard $(SRC_DIR)/app/*.cpp) \
        $(wildcard $(SRC_DIR)/core/math/*.cpp) \
        $(wildcard $(SRC_DIR)/loaders/*.cpp) \
        $(wildcard $(SRC_DIR)/renderers/cpu/*.cpp) \
        $(wildcard $(SRC_DIR)/geometry/*.cpp) \
        $(wildcard $(SRC_DIR)/math/*.cpp) \
        $(wildcard $(SRC_DIR)/scene/*.cpp)
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# ==============================================================================
# Shell command compatibility
# MSYS2 defines MSYSTEM, normal Windows CMD/PowerShell usually does not.
# ==============================================================================
ifdef MSYSTEM
	MKDIR_P = mkdir -p
	REMOVE_BUILD = rm -rf $(BUILD_DIR)
	REMOVE_TARGET = rm -f $(TARGET)
	RUN_TARGET = ./$(TARGET)
else
	MKDIR_P = powershell -Command New-Item -ItemType Directory -Force
	REMOVE_BUILD = if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	REMOVE_TARGET = if exist $(TARGET) del /f /q $(TARGET)
	RUN_TARGET = $(TARGET)
endif

# ==============================================================================
# Build Rules
# ==============================================================================
.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	@echo Linking executable: $@
	$(CXX) $(CXXFLAGS) $^ -o $@

# This rule pattern automatically maps any source file path to the build folder
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo Compiling: $<
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	$(RUN_TARGET)

clean:
	@echo Cleaning build artifacts...
	$(REMOVE_BUILD)
	$(REMOVE_TARGET)

-include $(DEPS)
