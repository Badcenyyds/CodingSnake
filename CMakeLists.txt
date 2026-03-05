# ==============================================================================
# CMakeLists.txt for CodingSnake Manual Play Client
# ==============================================================================
cmake_minimum_required(VERSION 3.16)
project(SnakeGame VERSION 1.0 LANGUAGES CXX)

# ==================== C++ 标准配置 ====================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==================== 编译选项配置 ====================
# 预编译头文件
option(USE_PCH "Use precompiled headers" ON)

# 编译优化选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wno-missing-field-initializers")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")

# 默认构建类型为 Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ==================== 目录配置 ====================
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/include)
set(RAYLIB_DIR ${CMAKE_SOURCE_DIR}/raylib/raylib-5.5_win64_mingw-w64)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# ==================== 查找源文件 ====================
# 自动检索 src 目录下所有 .cpp 文件
file(GLOB CLIENT_SOURCES "${SRC_DIR}/*.cpp")
file(GLOB HEADERS "${INC_DIR}/*.hpp")

# ==================== 可执行文件：可视化客户端 ====================
add_executable(${PROJECT_NAME} ${CLIENT_SOURCES} ${HEADERS})

# 添加头文件搜索路径
target_include_directories(${PROJECT_NAME} PRIVATE
    ${INC_DIR}
    ${RAYLIB_DIR}/include
    ${CMAKE_SOURCE_DIR}
)

# ==================== 复制配置文件到输出目录 ====================
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/config.cfg"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config.cfg"
    COMMENT "Copying config.cfg to output directory..."
)

# ==================== 链接库 ====================
link_directories(${RAYLIB_DIR}/lib)

target_link_libraries(${PROJECT_NAME} PRIVATE
    raylib
    ws2_32
    opengl32
    gdi32
    user32
    winmm
    pthread
)

# ==================== 编译定义 ====================
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

# ==================== 打印配置信息 ====================
message(STATUS "========================================")
message(STATUS "SnakeGame Configuration")
message(STATUS "========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "========================================")
