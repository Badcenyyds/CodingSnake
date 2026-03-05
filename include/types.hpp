/**
 * @file types.hpp
 * @brief 独立数据类型定义 - 不依赖任何第三方库
 *
 * 此文件包含游戏核心数据结构的独立定义：
 * - Point: 2D 坐标点（整数）
 * - Snake: 蛇的数据结构
 *
 * 注意：如果已包含 CodingSnake.hpp，则跳过这些类型的定义
 */

#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <vector>
#include <cmath>

// ============================================================================
// Point 和 Snake 类型定义
// ============================================================================
// 如果 CODING_SNAKE_HPP 已定义，Point 和 Snake 已在其中定义
// 我们使用宏来跳过重复定义

#if !defined(CODING_SNAKE_HPP)

/**
 * @brief 2D 坐标点
 */
struct Point {
    int x;
    int y;

    Point() : x(0), y(0) {}
    Point(int x_, int y_) : x(x_), y(y_) {}

    /**
     * @brief 计算与另一点的曼哈顿距离
     */
    int distance(const Point& other) const {
        return std::abs(x - other.x) + std::abs(y - other.y);
    }

    /**
     * @brief 计算与另一点的欧几里得距离平方
     */
    int distanceSquared(const Point& other) const {
        int dx = x - other.x;
        int dy = y - other.y;
        return dx * dx + dy * dy;
    }

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point& other) const {
        return !(*this == other);
    }

    bool operator<(const Point& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

/**
 * @brief 蛇（玩家）数据结构
 */
struct Snake {
    std::string id;                  // 玩家 ID
    std::string name;                // 玩家名字
    std::string color;               // 蛇的颜色
    Point head;                      // 头部位置
    std::vector<Point> blocks;       // 所有蛇身块（blocks[0] 是头部）
    int length;                      // 蛇的长度
    int invincible_rounds;           // 剩余无敌回合数

    Snake() : length(0), invincible_rounds(0) {}

    /**
     * @brief 检查某个位置是否在蛇身上
     */
    bool contains(const Point& p) const {
        for (const auto& block : blocks) {
            if (block == p) return true;
        }
        return false;
    }

    /**
     * @brief 检查蛇是否处于无敌状态
     */
    bool isInvincible() const {
        return invincible_rounds > 0;
    }
};

#endif // !CODING_SNAKE_HPP

#endif // TYPES_HPP
