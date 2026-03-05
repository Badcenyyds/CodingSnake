/**
 * @file pathfinder.hpp
 * @brief A* 路径规划模块 - 带权重避障的蛇路径搜索
 *
 * 功能：
 * - 使用 A* 算法搜索到最近食物的路径
 * - 根据与蛇头/身体/墙壁的距离给予权重惩罚
 * - 在权重最小的前提下最小化距离
 */

#ifndef PATHFINDER_HPP
#define PATHFINDER_HPP

// 使用 CodingSnake.hpp 中的类型定义（如果已包含）
// 否则使用 types.hpp
#if defined(CODING_SNAKE_HPP)
// 类型已在 CodingSnake.hpp 中定义
#else
#include "types.hpp"
#endif
#include "common.hpp"
#include "state.hpp"

#include <vector>
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include <cstdint>

// 权重配置
static constexpr float WEIGHT_COLLISION = 1e8;  // 碰撞（距离 0）

/**
 * @brief 预处理距离信息（可达距离，会被蛇身阻挡）
 */
std::vector<std::vector<int>> precomputeDistances(
    const std::vector<Snake>& snakes,
    const std::string& myId, int mode=0);

/**
 * @brief 预处理整个地图的权重，返回二维权重网格
 */
std::vector<std::vector<float>> precomputeWeights(
    const std::vector<Snake>& snakes,
    const std::string& myId);

/**
 * @brief 获取位置的权重（从预计算的网格中直接读取）
 */
float getCellWeight(const std::vector<std::vector<float>>& weights,
                    const Point& pos);

/**
 * @brief 获取四个方向的邻居
 */
std::vector<Point> getNeighbors(const Point& pos);

/**
 * @brief 检查位置是否有效（在地图范围内）
 */
bool isValid(const Point& pos);

/**
 * @brief 生成位置的唯一 key
 */
int makeKey(const Point& pos);

/**
 * @brief 寻路线程函数
 */
void pathPlanning(GameStateManager& state);

/**
 * @brief A* 搜索路径（返回路径）
 */
std::pair<float, std::vector<Point>> findPath(
    const Point& start,
    const Point& target,
    const std::vector<std::vector<float>>& weights);

/**
 * @brief Fallback: 找到一个安全的下一步（不保证到达食物）
 */
std::vector<Point> findSafeNextStep(
    const Point& startPos,
    const std::vector<std::vector<float>>& weights);

/**
 * @brief 获取下一步的移动方向
 * @return "up", "down", "left", "right" 或空字符串（无路径）
 */
std::string getNextMove(
    const Point& startPos,
    const std::vector<Point>& path);

#endif // PATHFINDER_HPP
