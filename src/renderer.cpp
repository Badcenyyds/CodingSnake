/**
 * @file renderer.cpp
 * @brief 渲染模块实现
 *
 * 使用 raylib 进行游戏图形渲染
 *
 * 注意：此文件包含 raylib.h，因此不应与 Windows API 直接混用
 */

#include "raylib.h"
#include "raymath.h"

#include "renderer.hpp"
#include "pathfinder.hpp"
#include <sstream>
#include <iomanip>

// ============================================================================
// 类型转换：将自定义类型转换为 raylib 类型
// ============================================================================
inline ::Color toRaylibColor(const config::colors::Color& c) {
    return ::Color{c.r, c.g, c.b, c.a};
}

// ============================================================================
// PIMPL 实现结构体 - 封装所有 raylib 相关数据和逻辑
// ============================================================================

struct Renderer::Impl {
    Camera2D camera;
    float targetZoom;
    int screenWidth;
    int screenHeight;

    Impl(int width, int height) 
        : screenWidth(width), screenHeight(height) {
        camera.target = {25.0f, 25.0f};
        camera.offset = {static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f};
        camera.rotation = 0.0f;
        camera.zoom = targetZoom = 10.0f;
    }

    /**
     * @brief 绘制网格背景
     */
    void drawGrid(int /*mapWidth*/, int /*mapHeight*/) {
        ::Color gridColor = toRaylibColor(config::colors::COLOR_GRID);

        int startX = 0;
        int startY = 0;
        int endX = 101;
        int endY = 101;

        // 绘制垂直线（直接使用世界坐标，raylib 会自动转换）
        for (int x = startX; x < endX; x++) {
            ::DrawLine(x, startY, x, endY, gridColor);
        }

        // 绘制水平线
        for (int y = startY; y < endY; y++) {
            ::DrawLine(startX, y, endX, y, gridColor);
        }
    }

    /**
     * @brief 可视化网格权重
     */
    void drawWeight(int mapWidth, int mapHeight,
                    std::vector<Snake> snakes,
                    std::string myId,
                    bool isUpdated, int mode) {
        static std::vector<std::vector<int>> distOt, distMe;
        static std::vector<std::vector<float>> weights;
        if(distOt.empty() || isUpdated)
            distOt = precomputeDistances(snakes, myId);
        if(distMe.empty() || isUpdated)
            distMe = precomputeDistances(snakes, myId, 1);
        if(weights.empty() || isUpdated)
            weights = precomputeWeights(snakes, myId);
        if(mode == 0) return ;
        for(int i = 0; i < mapWidth; ++i) {
            for(int j = 0; j < mapHeight; ++j) {
                int a; ::Color color;
                if(mode == 1) {
                    int weight = distMe[i][j] - distOt[i][j];
                    a = std::min(128, (int)(8 * pow(1.05, std::abs(weight))));
                    if(weight < 0) {
                        color = {0, 255, 0, static_cast<unsigned char>(a)};
                    } else {
                        color = {255, 0, 0, static_cast<unsigned char>(a)};
                    }
                } else if(mode == 2) {
                    a = weights[i][j] == WEIGHT_COLLISION ? 128 : 0;
                    color = {0, 0, 255, static_cast<unsigned char>(a)};
                }
                ::Rectangle rect = {
                    static_cast<float>(i),
                    static_cast<float>(j),
                    1.f, 1.f
                };
                DrawRectangleRec(rect, color);
            }
        }
    }

    /**
     * @brief 绘制蛇
     */
    void drawSnake(const Snake& snake, const config::colors::Color& color, int isme=0) {
        if (snake.blocks.empty()) return;

        float tileSize = config::BASE_TILE_SIZE;
        float padding = tileSize * 0.1f;

        // 绘制蛇的每个块（直接使用世界坐标）
        for (size_t i = 0; i < snake.blocks.size(); i++) {
            const Point& block = snake.blocks[i];
            float screenX = static_cast<float>(block.x);
            float screenY = static_cast<float>(block.y);

            ::Color raylibColor = toRaylibColor(color);
            if(isme) {
                raylibColor = ColorFromHSV((GetTime() * 20.f + i * 5.f), 0.8f, 0.8f);
            }

            // 蛇头稍大一些，身体稍小
            float padding_ = (i == 0) ? 0 : padding;

            ::Rectangle rect = {
                screenX + padding_,
                screenY + padding_,
                tileSize - 2 * padding_,
                tileSize - 2 * padding_
            };

            if (i == 0) {
                // 绘制蛇头（实心矩形）
                ::DrawRectangleRec(rect, raylibColor);

                // 绘制眼睛
                float eyeSize = tileSize * 0.12f;
                float eyeOffset = tileSize * 0.25f;
                ::Color eyeColor = {0, 0, 0, 255};

                // 根据移动方向确定眼睛位置
                Vector2 eye1Pos, eye2Pos;
                if (snake.length > 1 && snake.blocks.size() > 1) {
                    int dx = snake.blocks[0].x - snake.blocks[1].x;
                    int dy = snake.blocks[0].y - snake.blocks[1].y;

                    if (dx > 0) {  // 向右
                        eye1Pos = {rect.x + rect.width - eyeOffset, rect.y + eyeOffset * 0.5f};
                        eye2Pos = {rect.x + rect.width - eyeOffset, rect.y + rect.height - eyeOffset * 0.5f};
                    } else if (dx < 0) {  // 向左
                        eye1Pos = {rect.x + eyeOffset, rect.y + eyeOffset * 0.5f};
                        eye2Pos = {rect.x + eyeOffset, rect.y + rect.height - eyeOffset * 0.5f};
                    } else if (dy > 0) {  // 向下
                        eye1Pos = {rect.x + eyeOffset * 0.5f, rect.y + rect.height - eyeOffset};
                        eye2Pos = {rect.x + rect.width - eyeOffset * 0.5f, rect.y + rect.height - eyeOffset};
                    } else {  // 向上
                        eye1Pos = {rect.x + eyeOffset * 0.5f, rect.y + eyeOffset};
                        eye2Pos = {rect.x + rect.width - eyeOffset * 0.5f, rect.y + eyeOffset};
                    }
                } else {
                    // 默认向右
                    eye1Pos = {rect.x + rect.width - eyeOffset, rect.y + eyeOffset * 0.5f};
                    eye2Pos = {rect.x + rect.width - eyeOffset, rect.y + rect.height - eyeOffset * 0.5f};
                }

                ::DrawCircleV(eye1Pos, eyeSize, eyeColor);
                ::DrawCircleV(eye2Pos, eyeSize, eyeColor);
            } else {
                // 绘制身体（带边框的矩形）
                ::DrawRectangleRec(rect, raylibColor);
            }
        }
    }

    /**
     * @brief 绘制所有食物（绿色圆点）
     */
    void drawFoods(const std::vector<Point>& foods) {
        float tileSize = config::BASE_TILE_SIZE;
        float foodRadius = tileSize * 0.4f;
        ::Color foodColor = toRaylibColor(config::colors::COLOR_FOOD);

        for (const auto& food : foods) {
            // 直接使用世界坐标，raylib 会自动转换
            ::DrawCircleV(
                { static_cast<float>(food.x) + 0.5f, static_cast<float>(food.y) + 0.5f },
                foodRadius, foodColor
            );
        }
    }

    /**
     * @brief 绘制移动预览（下一步的位置预测）
     */
    void drawMovePreview(const Snake& mySnake, const std::string& move) {
        if (mySnake.blocks.empty()) return;

        float headX = static_cast<float>(mySnake.head.x);
        float headY = static_cast<float>(mySnake.head.y);
        float nextX = headX;
        float nextY = headY;

        // 计算下一步位置
        if (move == "up") nextY -= 1;
        else if (move == "down") nextY += 1;
        else if (move == "left") nextX -= 1;
        else if (move == "right") nextX += 1;

        float tileSize = config::BASE_TILE_SIZE;

        ::Rectangle previewRect = {
            nextX, nextY, tileSize, tileSize
        };

        // 半透明黄色预览框
        ::Color previewColor = toRaylibColor(config::colors::COLOR_PREVIEW);
        ::DrawRectangleRec(previewRect, previewColor);
        // ::DrawRectangleLinesEx(previewRect, 2, toRaylibColor(config::colors::COLOR_OWN_SNAKE));
    }

    /**
     * @brief 绘制规划路径（半透明绿色线条连接的路径点）
     */
    void drawPlannedPath(const std::vector<Point>& path) {
        if (path.size() < 2) return;

        float tileSize = config::BASE_TILE_SIZE;
        // 半透明绿色
        ::Color pathColor = {255, 192, 192, 80};

        // 绘制路径点之间的连线
        // 绘制路径点（小圆点）
        float pointRadius = tileSize * 0.25f;
        for (size_t i = 0; i < path.size(); ++i) {
            float x = static_cast<float>(path[i].x) + 0.5f;
            float y = static_cast<float>(path[i].y) + 0.5f;

            ::DrawCircleV({x, y}, pointRadius, pathColor);
        }
    }

    /**
     * @brief 绘制 UI 界面
     */
    void drawUI(const GameStateManager& state, bool showDebugInfo) {
        int y = 10;
        int x = 10;
        int lineHeight = 20;

        // 标题
        ::DrawText("CodingSnake - Manual Play", x, y, 20,
                   toRaylibColor(config::colors::COLOR_TEXT));
        y += lineHeight + 5;

        // 连接状态
        std::string status = state.isAuthenticated() ? "Connected" : "Disconnected";
        ::Color statusColor = state.isAuthenticated() ? GREEN : RED;
        ::DrawText(("Status: " + status).c_str(), x, y, 16, statusColor);
        y += lineHeight;

        // 游戏状态
        if (state.hasValidState()) {
            ::DrawText(("Direction: " + state.getPendingMove()).c_str(), x, y, 16,
                      toRaylibColor(config::colors::COLOR_TEXT));
            y += lineHeight;

            // 自动导航状态
            std::string navStatus = state.isAutoNavigate() ? "ON" : "OFF";
            ::Color navColor = state.isAutoNavigate() ? LIME : RED;
            ::DrawText(("Auto-Nav: " + navStatus).c_str(), x, y, 16, navColor);
            y += lineHeight;

            // 路径长度
            std::vector<Point> path = state.getPlannedPath();
            if (!path.empty()) {
                ::DrawText(("Path Length: " + std::to_string(path.size() - 1)).c_str(),
                          x, y, 16, toRaylibColor(config::colors::COLOR_TEXT));
                y += lineHeight;
            }

            Snake mySnake = state.getMySnake();
            ::DrawText(("Length: " + std::to_string(mySnake.length)).c_str(), x, y, 16,
                      toRaylibColor(config::colors::COLOR_TEXT));
            y += lineHeight;

            std::vector<Snake> snakes = state.getSnakes();
            ::DrawText(("Snakes: " + std::to_string(snakes.size())).c_str(), x, y, 16,
                      toRaylibColor(config::colors::COLOR_TEXT));
            y += lineHeight;

            std::vector<Point> foods = state.getFoods();
            ::DrawText(("Foods: " + std::to_string(foods.size())).c_str(), x, y, 16,
                      toRaylibColor(config::colors::COLOR_TEXT));
        } else {
            ::DrawText("Waiting for game state...", x, y, 16,
                      toRaylibColor(config::colors::COLOR_TEXT_DIM));
            y += lineHeight;
        }

        y += 15;

        // 控制说明
        ::DrawText("Controls:", x, y, 18, toRaylibColor(config::colors::COLOR_TEXT));
        y += lineHeight;
        ::DrawText("W/A/S/D - Change direction (auto-send)", x, y, 14,
                  toRaylibColor(config::colors::COLOR_TEXT_DIM));
        y += lineHeight;
        ::DrawText("N - Toggle auto-navigation", x, y, 14,
                  toRaylibColor(config::colors::COLOR_TEXT_DIM));
        y += lineHeight;
        ::DrawText("Mouse Wheel - Zoom in/out", x, y, 14,
                  toRaylibColor(config::colors::COLOR_TEXT_DIM));
        y += lineHeight;
        ::DrawText("F3 - Toggle debug info", x, y, 14,
                  toRaylibColor(config::colors::COLOR_TEXT_DIM));

        DrawFPS(screenWidth-80, screenHeight-20);
        // 调试信息（右上角）
        if (showDebugInfo) {
            int debugX = screenWidth - 250;
            int debugY = 10;

            // 调试面板背景
            ::DrawRectangleLinesEx(::Rectangle{
                    static_cast<float>(debugX - 5),
                    static_cast<float>(debugY - 5),
                    260.0f, 120.0f}, 2,
                    toRaylibColor(config::colors::COLOR_GRID));

            ::DrawText("Debug Info:", debugX, debugY, 16,
                      toRaylibColor(config::colors::COLOR_TEXT));
            debugY += lineHeight;

            std::ostringstream zoomStr;
            zoomStr << "Zoom: " << std::fixed << std::setprecision(2) << camera.zoom;
            ::DrawText(zoomStr.str().c_str(), debugX, debugY, 12,
                      toRaylibColor(config::colors::COLOR_TEXT_DIM));
            debugY += 16;

            ::DrawText(("FPS: " + std::to_string(::GetFPS())).c_str(), debugX, debugY, 12,
                      toRaylibColor(config::colors::COLOR_TEXT_DIM));
            debugY += 16;

            ::DrawText(("Map: " + std::to_string(state.getMapWidth()) + "x" +
                       std::to_string(state.getMapHeight())).c_str(), debugX, debugY, 12,
                      toRaylibColor(config::colors::COLOR_TEXT_DIM));
        }
    }
};

// ============================================================================
// Renderer 实现
// ============================================================================

Renderer::Renderer() : impl_(std::make_unique<Impl>(screenWidth_, screenHeight_)) {
}

Renderer::~Renderer() = default;

bool Renderer::initialize() {
    // 初始化 raylib 窗口
    ::InitWindow(screenWidth_, screenHeight_, config::WINDOW_TITLE);
    ::SetTargetFPS(config::TARGET_FPS);

    // 重新设置相机 offset（确保在窗口创建后）
    impl_->camera.offset = {static_cast<float>(screenWidth_) / 2.0f, 
                            static_cast<float>(screenHeight_) / 2.0f};

    bool success = ::IsWindowReady();
    if (success) {
        LOG_INFO("Renderer initialized: " << screenWidth_ << "x" << screenHeight_);
    } else {
        LOG_ERROR("Failed to initialize renderer");
    }

    return success;
}

bool Renderer::windowShouldClose() const {
    return ::WindowShouldClose();
}

void Renderer::beginDraw() {
    ::BeginDrawing();
    ::ClearBackground(toRaylibColor(config::colors::COLOR_BG));
}

void Renderer::endDraw() {
    ::EndDrawing();
}

Vector2 Renderer::screenToWorld(Vector2 screenPos) {
    return ::GetScreenToWorld2D(screenPos, impl_->camera);
}

void Renderer::handleZoom(float wheelDelta) {
    if (wheelDelta != 0) {
        impl_->targetZoom *= exp(wheelDelta * config::ZOOM_SPEED);
        // 限制缩放范围
        impl_->targetZoom = std::max(config::MIN_ZOOM, std::min(config::MAX_ZOOM, impl_->targetZoom));
    }
}

void Renderer::updateCamera(const GameStateManager& state) {
    // 平滑缩放
    impl_->camera.zoom += (impl_->targetZoom - impl_->camera.zoom) * 0.1f;

    // 跟随自己的蛇头
    if (state.hasValidState()) {
        Snake mySnake = state.getMySnake();
        if (!mySnake.blocks.empty()) {
            // 相机目标应该是格子中心（+0.5f）
            ::Vector2 targetPos = {
                static_cast<float>(mySnake.head.x) + 0.5f,
                static_cast<float>(mySnake.head.y) + 0.5f
            };
            // 平滑跟随
            impl_->camera.target += (targetPos - impl_->camera.target) *
                                   config::CAMERA_LERP_SPEED * ::GetFrameTime();
        }
    } else {
        // 没有有效状态时，回到地图中心
        ::Vector2 centerPos = {25.5f, 25.5f};
        impl_->camera.target += (centerPos - impl_->camera.target) *
                               config::CAMERA_LERP_SPEED * ::GetFrameTime();
    }
}

void Renderer::render(const GameStateManager& state) {
    static int lastRound = -1;
    int isUpdated = state.isUpdated(lastRound);

    // 绘制游戏世界
    ::BeginMode2D(impl_->camera);

    if (state.hasValidState()) {
        int mapWidth = state.getMapWidth();
        int mapHeight = state.getMapHeight();

        // 绘制网格
        impl_->drawGrid(mapWidth, mapHeight);

        // 绘制食物（绿色圆点）
        impl_->drawFoods(state.getFoods());

        // 绘制其他蛇（随机颜色）
        std::vector<Snake> snakes = state.getSnakes();
        std::string playerId = state.getPlayerId();
        for (const auto& snake : snakes) {
            if (snake.id != playerId) {
                config::colors::Color color = state.getSnakeColor(snake.id, false);
                impl_->drawSnake(snake, color);
            }
        }

        // 绘制自己的蛇（白色）
        Snake mySnake = state.getMySnake();
        impl_->drawSnake(mySnake, config::colors::COLOR_OWN_SNAKE, 1);

        // 绘制移动预览
        impl_->drawMovePreview(mySnake, state.getPendingMove());

        if(state.isAutoNavigate()) {
            // 绘制规划路径（半透明绿色）
            impl_->drawPlannedPath(state.getPlannedPath());
        }

        // 可视化权重
        impl_->drawWeight(mapWidth, mapHeight, snakes, playerId, isUpdated, debugMode_ % 3);
    } else {
        // 等待状态时绘制空网格
        impl_->drawGrid(50, 50);

        const char* waitText = "Waiting for game state...";
        int waitWidth = ::MeasureText(waitText, 20);
        ::DrawText(waitText, screenWidth_ / 2 - waitWidth / 2,
                  screenHeight_ / 2 - 10, 20, toRaylibColor(config::colors::COLOR_TEXT));
    }

    ::EndMode2D();

    // 绘制 UI
    impl_->drawUI(state, debugMode_ % 2);
}

void Renderer::drawPath(const std::vector<Point>& path) {
    impl_->drawPlannedPath(path);
}

void Renderer::shutdown() {
    ::ShowCursor();  // 显示鼠标
    ::CloseWindow();
    LOG_INFO("Renderer shutdown");
}
