/**
 * @file common.hpp
 * @brief 公共头文件 - 包含类型定义、工具函数和配置常量
 *
 * 此文件被所有其他模块包含，提供：
 * - 标准库的引入
 * - 项目配置常量
 * - 通用工具函数
 * - 类型别名
 *
 * 注意：此文件不依赖任何第三方库（无 json，无 raylib，无 windows.h）
 */

#ifndef COMMON_HPP
#define COMMON_HPP

// ============================================================================
// 标准库头文件
// ============================================================================
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <random>

// ============================================================================
// 随机数生成器
// ============================================================================
namespace RndNumGen {
    extern std::mt19937_64 rng;
};

// ============================================================================
// 游戏方向枚举和工具函数
// ============================================================================

/**
 * @brief 游戏方向枚举
 */
enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
};

/**
 * @brief 方向字符串转换工具
 */
namespace DirectionUtils {
    inline std::string toString(Direction dir) {
        switch (dir) {
            case Direction::UP: return "up";
            case Direction::DOWN: return "down";
            case Direction::LEFT: return "left";
            case Direction::RIGHT: return "right";
            default: return "right";
        }
    }

    inline Direction fromString(const std::string& str) {
        if (str == "up") return Direction::UP;
        if (str == "down") return Direction::DOWN;
        if (str == "left") return Direction::LEFT;
        if (str == "right") return Direction::RIGHT;
        return Direction::RIGHT;
    }

    /**
     * @brief 检查新方向是否与当前方向相反（相反方向无效）
     */
    inline bool isOpposite(Direction current, Direction newDir) {
        if (current == Direction::UP && newDir == Direction::DOWN) return true;
        if (current == Direction::DOWN && newDir == Direction::UP) return true;
        if (current == Direction::LEFT && newDir == Direction::RIGHT) return true;
        if (current == Direction::RIGHT && newDir == Direction::LEFT) return true;
        return false;
    }
}

// ============================================================================
// 配置常量（默认值，可被配置文件覆盖）
// ============================================================================

namespace config {
    // 服务器地址
    inline const char* SERVER_URL = "http://csapi.seveoi.icu:18080";

    // 窗口设置
    inline int SCREEN_WIDTH = 1280;
    inline int SCREEN_HEIGHT = 720;
    inline const char* WINDOW_TITLE = "CodingSnake";

    // 渲染设置
    inline float BASE_TILE_SIZE = 1.0f;     // 基础格子大小（像素）
    inline float MIN_ZOOM = 3.0f;            // 最小缩放
    inline float MAX_ZOOM = 100.0f;          // 最大缩放
    inline float ZOOM_SPEED = 0.05f;         // 缩放速度
    inline float CAMERA_LERP_SPEED = 5.0f;   // 相机跟随速度
    inline int TARGET_FPS = 60;              // 目标帧率

    // 游戏参数
    inline int MAP_WIDTH = 100;
    inline int MAP_HEIGHT = 100;

    // 网络轮询间隔（毫秒）
    inline int NETWORK_POLL_INTERVAL_MS = 250;

    // 颜色配置（保持不变，使用 constexpr）
    namespace colors {
        struct Color {
            unsigned char r, g, b, a;
        };

        inline constexpr Color COLOR_BG = {63, 63, 64, 255};
        inline constexpr Color COLOR_GRID = {80, 80, 100, 255};
        inline constexpr Color COLOR_OWN_SNAKE = {255, 255, 255, 255};
        inline constexpr Color COLOR_FOOD = {0, 255, 100, 255};
        inline constexpr Color COLOR_PREVIEW = {255, 255, 255, 128};
        inline constexpr Color COLOR_TEXT = {220, 220, 220, 255};
        inline constexpr Color COLOR_TEXT_DIM = {150, 150, 150, 255};
    }

    /**
     * @brief 重置配置为默认值
     */
    inline void resetToDefaults() {
        SERVER_URL = "http://csapi.seveoi.icu:18080";
        SCREEN_WIDTH = 1280;
        SCREEN_HEIGHT = 720;
        WINDOW_TITLE = "CodingSnake - Manual Play";
        BASE_TILE_SIZE = 1.0f;
        MIN_ZOOM = 3.0f;
        MAX_ZOOM = 100.0f;
        ZOOM_SPEED = 0.05f;
        CAMERA_LERP_SPEED = 5.0f;
        TARGET_FPS = 60;
        MAP_WIDTH = 100;
        MAP_HEIGHT = 100;
        NETWORK_POLL_INTERVAL_MS = 250;
    }
}

// ============================================================================
// 工具函数
// ============================================================================

/**
 * @brief 生成随机颜色（HSL 转 RGB）
 */
inline config::colors::Color generateRandomColor() {
    float h = static_cast<float>(std::rand() % 360) / 360.0f;
    float s = 0.7f + static_cast<float>(std::rand() % 30) / 100.0f;
    float v = 0.8f + static_cast<float>(std::rand() % 20) / 100.0f;

    int hi = static_cast<int>(h * 6.0f) % 6;
    float f = h * 6.0f - hi;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);

    config::colors::Color result;
    result.a = 255;

    switch (hi) {
        case 0: result.r = static_cast<unsigned char>(v * 255); result.g = static_cast<unsigned char>(t * 255); result.b = static_cast<unsigned char>(p * 255); break;
        case 1: result.r = static_cast<unsigned char>(q * 255); result.g = static_cast<unsigned char>(v * 255); result.b = static_cast<unsigned char>(p * 255); break;
        case 2: result.r = static_cast<unsigned char>(p * 255); result.g = static_cast<unsigned char>(v * 255); result.b = static_cast<unsigned char>(t * 255); break;
        case 3: result.r = static_cast<unsigned char>(p * 255); result.g = static_cast<unsigned char>(q * 255); result.b = static_cast<unsigned char>(v * 255); break;
        case 4: result.r = static_cast<unsigned char>(t * 255); result.g = static_cast<unsigned char>(p * 255); result.b = static_cast<unsigned char>(v * 255); break;
        case 5: result.r = static_cast<unsigned char>(v * 255); result.g = static_cast<unsigned char>(p * 255); result.b = static_cast<unsigned char>(q * 255); break;
    }

    return result;
}

/**
 * @brief 格式化浮点数
 */
inline std::string formatFloat(float value, int precision = 1) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

/**
 * @brief 日志输出宏（可在此处添加时间戳等）
 */
#define LOG_INFO(msg) std::cout << "[INFO] " << msg << std::endl
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl
#define LOG_DEBUG(msg) std::cout << "[DEBUG] " << msg << std::endl

#endif // COMMON_HPP
