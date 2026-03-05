/**
 * @file renderer.hpp
 * @brief 渲染模块 - 使用 raylib 进行图形渲染
 *
 * 功能：
 * - 初始化/关闭 raylib 窗口
 * - 绘制游戏世界（网格、蛇、食物）
 * - 绘制 UI（状态信息、控制提示）
 * - 相机管理（缩放、跟随）
 *
 * 运行在主线程中（与渲染在同一线程）
 *
 * 注意：此头文件不直接包含 raylib.h，依赖被封装在 .cpp 文件中
 */

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "state.hpp"
#include <memory>

// 前向声明 raylib 的 Vector2
struct Vector2;

/**
 * @brief 渲染管理器
 *
 * 封装所有 raylib 相关的渲染逻辑
 */
class Renderer {
private:
    // PIMPL 模式：将 raylib 相关的实现完全封装在 .cpp 中
    struct Impl;
    std::unique_ptr<Impl> impl_;

    // ==================== 窗口设置 ====================
    int screenWidth_ = config::SCREEN_WIDTH;
    int screenHeight_ = config::SCREEN_HEIGHT;

    // ==================== 调试信息 ====================
    int debugMode_ = 0;

public:
    /**
     * @brief 构造函数
     */
    Renderer();

    ~Renderer();

    /**
     * @brief 初始化渲染器（创建窗口等）
     * @return true 成功，false 失败
     */
    bool initialize();

    /**
     * @brief 检查窗口是否应该关闭
     */
    bool windowShouldClose() const;

    /**
     * @brief 开始绘制一帧
     */
    void beginDraw();

    /**
     * @brief 结束绘制并显示
     */
    void endDraw();

    /**
     * @brief 更新相机（跟随蛇头、缩放）
     */
    void updateCamera(const GameStateManager& state);

    /**
     * @brief 处理缩放输入
     * @param wheelDelta 滚轮增量
     */
    void handleZoom(float wheelDelta);

    /**
     * @brief 切换调试信息显示
     */
    void toggleDebugMode() { ++debugMode_; }
    bool getDebugMode() const { return debugMode_; }

    /**
     * @brief 将屏幕坐标转换为世界坐标（用于鼠标交互）
     */
    Vector2 screenToWorld(Vector2 screenPos);

    /**
     * @brief 主渲染函数（绘制整个游戏画面）
     */
    void render(const GameStateManager& state);

    /**
     * @brief 绘制规划的路径
     */
    void drawPath(const std::vector<Point>& path);

    /**
     * @brief 关闭渲染器，释放资源
     */
    void shutdown();
};

#endif // RENDERER_HPP
