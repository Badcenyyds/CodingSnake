/**
 * @file input.cpp
 * @brief 输入处理模块实现
 *
 * 处理玩家键盘和鼠标输入，更新移动指令
 *
 * 注意：此文件包含 raylib.h，因此会引入 raylib 依赖
 */

#include "raylib.h"

#include "input.hpp"
#include "renderer.hpp"
#include "pathfinder.hpp"

// ============================================================================
// PIMPL 实现结构体
// ============================================================================

struct InputManager::Impl {
    // 此处可以放置需要与 raylib 输入相关的成员变量
    // 当前实现中，所有输入处理都直接使用 raylib API，不需要额外状态
};

// ============================================================================
// InputManager 实现
// ============================================================================

InputManager::InputManager(GameStateManager& state, Renderer& renderer)
    : state_(state), renderer_(renderer),
      impl_(std::make_unique<Impl>()) {
}

InputManager::~InputManager() = default;

void InputManager::handleDirectionKeys() {
    // WASD 或方向键控制方向 - 立即发送移动指令
    Direction newDirection = currentDirection_;  // 默认保持当前方向
    bool directionChanged = false;

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        newDirection = Direction::UP;
        directionChanged = (currentDirection_ != Direction::UP);
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        newDirection = Direction::DOWN;
        directionChanged = (currentDirection_ != Direction::DOWN);
    }
    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        newDirection = Direction::LEFT;
        directionChanged = (currentDirection_ != Direction::LEFT);
    }
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        newDirection = Direction::RIGHT;
        directionChanged = (currentDirection_ != Direction::RIGHT);
    }

    // 如果方向改变，立即发送
    if (directionChanged) {
        currentDirection_ = newDirection;
        std::string moveStr;
        switch (newDirection) {
            case Direction::UP: moveStr = "up"; break;
            case Direction::DOWN: moveStr = "down"; break;
            case Direction::LEFT: moveStr = "left"; break;
            case Direction::RIGHT: moveStr = "right"; break;
            default: moveStr = "right"; break;
        }
        state_.setPendingMove(moveStr);
    }
}

void InputManager::handleActionKeys() {
    // N 键：切换自动导航
    if (IsKeyPressed(KEY_N)) {
        bool currentlyEnabled = state_.isAutoNavigate();
        state_.setAutoNavigate(!currentlyEnabled);
    }

    // F3 键：切换调试信息
    if (IsKeyPressed(KEY_F3)) {
        renderer_.toggleDebugMode();
    }
}

void InputManager::update() {
    // 只有在认证成功且游戏状态有效时才处理输入
    if (!state_.isAuthenticated()) {
        return;
    }

    // 处理方向键
    handleDirectionKeys();

    // 处理功能键
    handleActionKeys();

    // 处理滚轮缩放（委托给 Renderer）
    float wheel = GetMouseWheelMove();
    renderer_.handleZoom(wheel);
}
