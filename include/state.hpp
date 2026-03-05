/**
 * @file state.hpp
 * @brief 游戏状态管理 - 线程安全的数据共享结构
 *
 * 负责：
 * - 存储游戏世界的所有状态（蛇、食物、地图等）
 * - 提供线程安全的读写接口
 * - 管理玩家认证信息
 * - 管理待发送的移动指令
 *
 * 注意：此文件不依赖任何平台特定头文件（无 windows.h，无 raylib.h）
 */

#ifndef STATE_HPP
#define STATE_HPP

#include "types.hpp"
#include "common.hpp"
#include <mutex>
#include <shared_mutex>
#include <map>
#include <vector>

/**
 * @brief 游戏状态管理器
 *
 * 此类被网络线程和渲染线程共享访问：
 * - 网络线程：写入状态
 * - 渲染线程：读取状态
 * - 输入线程：写入移动指令
 *
 * 使用读写锁（shared_mutex）优化性能：
 * - 多个渲染线程可同时读取
 * - 网络线程独占写入
 */
class GameStateManager {
private:
    // ==================== 锁机制 ====================
    mutable std::shared_mutex stateMutex_;  // 状态数据锁

    // ==================== 认证信息 ====================
    std::string playerId_;       // 当前玩家 ID
    bool isAuthenticated_ = false;

    // ==================== 游戏世界状态 ====================
    std::vector<Snake> snakes_;      // 所有蛇
    std::vector<Point> foods_;       // 所有食物
    int mapWidth_ = 100;              // 地图宽度
    int mapHeight_ = 100;             // 地图高度
    bool hasValidState_ = false;     // 是否有有效状态
    int currentRound_ = 0; // 当前回合

    // ==================== 颜色缓存 ====================
    // 为每条蛇缓存一个颜色，避免每帧重新生成
    // 使用 mutable 以便在 const 成员函数中修改
    mutable std::map<std::string, config::colors::Color> snakeColorCache_;

    // ==================== 移动指令 ====================
    std::string pendingMove_ = "right";   // 待发送的移动方向
    std::string lastSentMove_ = "";       // 上次发送的移动方向
    bool hasPendingMove_ = false;         // 是否有待发送的移动

    // ==================== 路径规划 ====================
    std::vector<Point> plannedPath_;    // 规划的路径
    bool autoNavigate_ = true;          // 是否启用自动导航

    // ==================== 线程控制 ====================
    std::atomic<bool> shouldStop_{false}; // 停止标志
    std::atomic<bool> isConnected_{false}; // 连接状态

public:
    // ==================== 构造/析构 ====================
    GameStateManager() = default;
    ~GameStateManager() = default;

    // 禁止拷贝，允许移动
    GameStateManager(const GameStateManager&) = delete;
    GameStateManager& operator=(const GameStateManager&) = delete;
    GameStateManager(GameStateManager&&) = default;
    GameStateManager& operator=(GameStateManager&&) = default;

    // ==================== 线程控制 ====================
    void requestStop() { shouldStop_.store(true); }
    bool shouldStop() const { return shouldStop_.load(); }
    void setConnected(bool connected) { isConnected_.store(connected); }
    bool isConnected() const { return isConnected_.load(); }

    // ==================== 认证信息管理 ====================

    /**
     * @brief 设置认证信息
     */
    void setAuthInfo(int flag) {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        // playerId_ = playerId;
        isAuthenticated_ = flag;
    }

    void setPlayerId(std::string playerid) {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        playerId_ = playerid;
    }

    std::string getPlayerId() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        return playerId_;
    }

    bool isAuthenticated() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        return isAuthenticated_;
    }

    // ==================== 游戏状态读取（线程安全） ====================

    /**
     * @brief 获取所有蛇的副本
     */
    std::vector<Snake> getSnakes() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        return snakes_;
    }

    /**
     * @brief 获取所有蛇的副本（别名，用于网络模块）
     */
    std::vector<Snake> getAllSnakes() const {
        return getSnakes();
    }

    /**
     * @brief 获取自己的蛇
     */
    Snake getMySnake() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        for (const auto& snake : snakes_) {
            if (snake.id == playerId_) {
                return snake;
            }
        }
        return Snake{};  // 返回空蛇
    }

    /**
     * @brief 获取所有食物
     */
    std::vector<Point> getFoods() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        return foods_;
    }

    /**
     * @brief 获取所有食物的副本（别名，用于网络模块）
     */
    std::vector<Point> getAllFoods() const {
        return getFoods();
    }

    int getMapWidth() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        return mapWidth_;
    }

    int getMapHeight() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        return mapHeight_;
    }

    bool hasValidState() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        return hasValidState_;
    }

    /**
     * @brief 获取地图是否更新
     */
    bool isUpdated(int& lastRound) const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        bool ret = lastRound != currentRound_;
        lastRound = currentRound_;
        return ret;
    }

    /**
     * @brief 获取蛇的颜色（带缓存）
     */
    config::colors::Color getSnakeColor(const std::string& snakeId, bool isOwn) const {
        if (isOwn) {
            return config::colors::COLOR_OWN_SNAKE;
        }

        // 先尝试读取（使用共享锁）
        {
            std::shared_lock<std::shared_mutex> lock(stateMutex_);
            auto it = snakeColorCache_.find(snakeId);
            if (it != snakeColorCache_.end()) {
                return it->second;
            }
        }

        // 缓存未命中，生成新颜色（使用独占锁）
        config::colors::Color color = generateRandomColor();
        std::unique_lock<std::shared_mutex> writeLock(stateMutex_);
        // 直接插入或返回已存在的
        auto result = snakeColorCache_.emplace(snakeId, color);
        return result.first->second;
    }

    // ==================== 游戏状态写入（线程安全） ====================

    /**
     * @brief 更新游戏状态（由网络线程调用）
     *
     * @param snakes 所有蛇的数据
     * @param foods 所有食物的数据
     * @param mapWidth 地图宽度
     * @param mapHeight 地图高度
     * @param round 当前回合
     */
    void updateGameState(const std::vector<Snake>& snakes,
                         const std::vector<Point>& foods,
                         int mapWidth, int mapHeight) {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        snakes_ = snakes;
        foods_ = foods;
        mapWidth_ = mapWidth;
        mapHeight_ = mapHeight;
        hasValidState_ = true;
    }

    /**
     * @brief 移除玩家（delta 更新用）
     */
    void removePlayer(const std::string& id) {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        snakes_.erase(
            std::remove_if(snakes_.begin(), snakes_.end(),
                [&id](const Snake& s) { return s.id == id; }),
            snakes_.end());
    }

    /**
     * @brief 添加或更新玩家（delta 更新用）
     */
    void addOrUpdatePlayer(const Snake& snake) {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        for (auto& s : snakes_) {
            if (s.id == snake.id) {
                s = snake;
                return;
            }
        }
        snakes_.push_back(snake);
    }

    /**
     * @brief 移除食物（delta 更新用）
     */
    void removeFood(const Point& food) {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        foods_.erase(
            std::remove_if(foods_.begin(), foods_.end(),
                [&food](const Point& f) { return f.x == food.x && f.y == food.y; }),
            foods_.end());
    }

    /**
     * @brief 添加食物（delta 更新用）
     */
    void addFood(const Point& food) {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        foods_.push_back(food);
    }

    /**
     * @brief 根据 ID 查找玩家（返回指针，线程不安全，调用者需保证线程安全）
     * @note 仅用于网络线程内部，在持有锁的情况下调用
     */
    Snake* findPlayerById(const std::string& id) {
        for (auto& s : snakes_) {
            if (s.id == id) {
                return &s;
            }
        }
        return nullptr;
    }

    /**
     * @brief 更新当前回合
     */
    void updateRound(int round) {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        currentRound_ = round;
    }

    // ==================== 移动指令管理 ====================

    /**
     * @brief 设置待发送的移动方向
     */
    void setPendingMove(const std::string& direction) {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        pendingMove_ = direction;
        hasPendingMove_ = true;
    }

    std::string getPendingMove() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        return pendingMove_;
    }

    void setLastSentMove(const std::string& direction) {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        lastSentMove_ = direction;
    }

    std::string getLastSentMove() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        return lastSentMove_;
    }

    bool hasPendingMove() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        return hasPendingMove_;
    }

    void clearPendingMove() {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        hasPendingMove_ = false;
    }

    // ==================== 路径规划管理 ====================

    void setPlannedPath(const std::vector<Point>& path) {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        plannedPath_ = path;
    }

    std::vector<Point> getPlannedPath() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        return plannedPath_;
    }

    void clearPlannedPath() {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        plannedPath_.clear();
    }

    void setAutoNavigate(bool enabled) {
        std::unique_lock<std::shared_mutex> lock(stateMutex_);
        autoNavigate_ = enabled;
    }

    bool isAutoNavigate() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        return autoNavigate_;
    }

    // ==================== 调试信息 ====================

    /**
     * @brief 获取状态摘要（用于调试显示）
     */
    std::string getStateSummary() const {
        std::shared_lock<std::shared_mutex> lock(stateMutex_);
        std::ostringstream oss;
        oss << " | Snakes: " << snakes_.size()
            << " | Foods: " << foods_.size()
            << " | Map: " << mapWidth_ << "x" << mapHeight_;
        return oss.str();
    }
};

#endif // STATE_HPP
