# ==============================================================================
# Makefile for CodingSnake Manual Play Client
# ==============================================================================
# 使用方法:
#   make          - 编译程序 (使用传统 make)
#   make cmake    - 使用 CMake 编译
#   make clean    - 清理编译产物
#   make run      - 编译并运行
#   make debug    - 编译调试版本
#
# 依赖:
#   - MinGW-w64 (g++)
#   - raylib 5.5
#   - CodingSnake.hpp
#   - CMake 3.16+ (可选，用于 cmake 目标)
#
# ==============================================================================

# ==================== 项目配置 ====================

# 项目名称
TARGET = snake_game

# C++ 标准
CXXSTD = -std=c++17

# 编译器
CXX = g++

# 编译器警告
WARNINGS = -Wall -Wextra -Wpedantic -Wno-missing-field-initializers

# ==================== 目录配置 ====================

# 源代码目录
SRC_DIR = src

# 头文件目录
INC_DIR = include

# 编译产物目录
OBJ_DIR = obj

# 构建目录（CMake）
BUILD_DIR = build

# 输出目录（CMake）
BIN_DIR = bin

# raylib 目录（根据你的实际安装位置修改）
RAYLIB_DIR = raylib/raylib-5.5_win64_mingw-w64
RAYLIB_INC = $(RAYLIB_DIR)/include
RAYLIB_LIB = $(RAYLIB_DIR)/lib

# ==================== 编译选项 ====================

# 头文件搜索路径
INCLUDES = -I$(INC_DIR) -I$(RAYLIB_INC) -I.

# 库文件搜索路径
LIBPATHS = -L$(RAYLIB_LIB)

# 链接的库
LIBS = -lraylib -lws2_32 -lopengl32 -lgdi32 -luser32 -lwinmm -lpthread

# 编译器标志
CXXFLAGS = $(CXXSTD) $(WARNINGS) $(INCLUDES) -O2 -MMD -MP

# 预编译头文件
PCH = include/pch.hpp
PCH_OUT = $(OBJ_DIR)/pch.hpp.gch

# 链接器标志
LDFLAGS = $(LIBPATHS) $(LIBS)

# ==================== 源文件 ====================
# 自动检索 src 目录下所有 .cpp 文件
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# 依赖文件
DEPS = $(OBJS:.o=.d)

# ==================== 默认目标 ====================

.PHONY: all clean clean-all run debug cmake cmake-debug help copy-config

all: $(PCH_OUT) $(TARGET).exe copy-config

# ==================== 传统 Make 编译规则 ====================

$(OBJ_DIR):
	if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)

# 编译预编译头文件
$(PCH_OUT): $(PCH) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -x c++-header $(PCH) -o $(PCH_OUT)

# 编译源文件为目标文件（依赖 PCH）
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(PCH_OUT) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -include pch.hpp -c $< -o $@

$(TARGET).exe: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@
	@echo "======================================"
	@echo "Build successful!"
	@echo "Run with: make run"
	@echo "======================================"

# 复制配置文件到输出目录
copy-config:
	if exist config.cfg copy /Y config.cfg $(TARGET).exe.config >nul 2>&1 || copy /Y config.cfg . >nul 2>&1

# ==================== 调试版本 ====================

debug: CXXFLAGS = $(CXXSTD) $(WARNINGS) $(INCLUDES) -g -DDEBUG
debug: clean $(TARGET)_debug.exe
	@echo "Debug build complete!"

$(TARGET)_debug.exe: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

# ==================== CMake 编译 ====================

cmake:
	@echo "======================================"
	@echo "Configuring with CMake..."
	@echo "======================================"
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
	cd $(BUILD_DIR) && cmake --build . --parallel 8
	@echo "======================================"
	@echo "CMake build complete!"
	@echo "Executable: $(BIN_DIR)\$(TARGET).exe"
	@echo "======================================"

cmake-debug:
	@echo "======================================"
	@echo "Configuring CMake Debug Build..."
	@echo "======================================"
	if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
	cd $(BUILD_DIR) && cmake --build . --parallel 8
	@echo "======================================"
	@echo "CMake Debug build complete!"
	@echo "Executable: $(BIN_DIR)\$(TARGET).exe"
	@echo "======================================"

cmake-clean:
	@echo "Cleaning CMake build artifacts..."
	if exist $(BUILD_DIR) rmdir /s /q $(BUILD_DIR)
	if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)
	@echo "CMake artifacts cleaned!"

# ==================== 运行程序 ====================

run: $(TARGET).exe
	.\$(TARGET).exe

run-cmake:
	if exist $(BIN_DIR)\$(TARGET).exe (.\$(BIN_DIR)\$(TARGET).exe) else (echo "CMake build not found. Run 'make cmake' first.")

# ==================== 清理 ====================

clean:
	if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR)
	if exist $(TARGET).exe del /q $(TARGET).exe
	if exist $(TARGET)_debug.exe del /q $(TARGET)_debug.exe
	if exist $(PCH_OUT) del /q $(PCH_OUT)
	@echo "Traditional make artifacts cleaned!"

clean-all: clean cmake-clean
	@echo "All build artifacts cleaned!"

# ==================== 依赖文件包含 ====================
# 自动包含生成的依赖文件（.d 文件）
-include $(DEPS)

# ==================== 帮助信息 ====================

help:
	@echo "======================================"
	@echo "CodingSnake Makefile Help"
	@echo "======================================"
	@echo ""
	@echo "Available targets:"
	@echo "  all (default) - Build using traditional make"
	@echo "  cmake         - Build using CMake (Release)"
	@echo "  cmake-debug   - Build using CMake (Debug)"
	@echo "  clean         - Clean traditional make artifacts"
	@echo "  cmake-clean   - Clean CMake artifacts"
	@echo "  clean-all     - Clean all build artifacts"
	@echo "  run           - Build and run (traditional make)"
	@echo "  run-cmake     - Run CMake build"
	@echo "  debug         - Build debug version (traditional make)"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Configuration:"
	@echo "  CXX         = $(CXX)"
	@echo "  CXXSTD      = $(CXXSTD)"
	@echo "  TARGET      = $(TARGET)"
	@echo "  BUILD_DIR   = $(BUILD_DIR)"
	@echo "  BIN_DIR     = $(BIN_DIR)"
	@echo ""
	@echo "Quick Start:"
	@echo "  make cmake    - First time build with CMake"
	@echo "  make run-cmake - Run the CMake-built executable"
	@echo "  make clean-all - Clean everything"
	@echo ""
