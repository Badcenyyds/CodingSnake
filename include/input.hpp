/**
 * @file input.hpp
 * @brief 输入处理模块 - 处理键盘和鼠标输入
 *
 * 功能：
 * - WASD/方向键控制移动方向
 * - 空格键锁定/解锁移动
 * - Enter 键立即提交移动
 * - 滚轮缩放（委托给 Renderer）
 * - F3 切换调试信息
 *
 * 运行在主线程中（与渲染在同一线程）
 *
 * 注意：此头文件不直接包含 raylib.h，依赖被封装在 .cpp 文件中
 */

#ifndef INPUT_HPP
#define INPUT_HPP

#include "state.hpp"
#include <memory>

// 前向声明
class Renderer;
class NetworkManager;

/**
 * @brief 输入管理器
 *
 * 处理所有玩家输入，并更新 GameStateManager 中的移动指令
 */
class InputManager {
private:
    GameStateManager& state_;  // 状态管理器（引用）
    Renderer& renderer_;        // 渲染器（引用）

    // PIMPL 模式：将 raylib 相关的实现完全封装在 .cpp 中
    struct Impl;
    std::unique_ptr<Impl> impl_;

    // 当前按下的方向键
    Direction currentDirection_ = Direction::RIGHT;

    /**
     * @brief 处理方向键输入
     */
    void handleDirectionKeys();

    /**
     * @brief 处理功能键输入
     */
    void handleActionKeys();

public:
    /**
     * @brief 构造函数
     */
    InputManager(GameStateManager& state, Renderer& renderer);

    ~InputManager();

    /**
     * @brief 更新输入状态（每帧调用）
     */
    void update();

    /**
     * @brief 获取当前选择的方向
     */
    Direction getCurrentDirection() const { return currentDirection_; }

    /**
     * @brief 设置当前方向
     */
    void setCurrentDirection(Direction dir) { currentDirection_ = dir; }
};

#endif // INPUT_HPP
