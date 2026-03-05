/**
 * @file main.cpp
 * @brief CodingSnake 手动游戏客户端 - 主程序入口
 *
 * 程序架构：
 * - 主线程：负责渲染和输入处理
 * - 网络线程：负责与服务器通信，轮询游戏状态
 * - GameStateManager：线程安全的数据共享结构
 *
 * 模块依赖关系（从底层到高层）：
 * 1. types.hpp      : 纯数据类型（Point, Snake, Vector2, Rectangle）
 * 2. common.hpp     : 配置常量、工具函数、Direction 枚举
 * 3. state.hpp      : 游戏状态管理（依赖 types.hpp, common.hpp）
 * 4. network.hpp    : 网络通信（依赖 state.hpp，Windows API 封装在 .cpp 中）
 * 5. renderer.hpp   : 渲染模块（依赖 types.hpp, state.hpp，raylib 封装在 .cpp 中）
 * 6. input.hpp      : 输入处理（依赖 state.hpp，raylib 封装在 .cpp 中）
 *
 * 使用方法：
 * 1. 修改 USER_ID 和 PASTE 为你的认证信息
 * 2. 运行程序
 * 3. 使用 WASD 控制蛇的移动方向
 *
 * 控制说明：
 * - W/A/S/D 或方向键：选择移动方向
 * - 空格：锁定/解锁移动
 * - Enter：立即提交移动
 * - 滚轮：缩放视图
 * - F3：切换调试信息
 */

// ============================================================================
// 头文件包含顺序：从底层到高层
// ============================================================================

// 2. 核心状态管理
#include "state.hpp"

// 3. 功能模块（这些头文件不包含平台特定依赖）
#include "network.hpp"
#include "renderer.hpp"
#include "input.hpp"
#include "pathfinder.hpp"

// 4. 配置解析器
#include "config_parser.hpp"

#include <thread>
#include <memory>

// ============================================================================
// 全局配置
// ============================================================================
static Config g_config;

// ============================================================================
// 全局日志：显示程序启动信息
// ============================================================================
void printStartupInfo() {
    std::cout << "======================================" << std::endl;
    std::cout << "  CodingSnake - Manual Play Client" << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << "User ID: " << g_config.user.userId << std::endl;
    std::cout << "Player Name: " << g_config.user.playerName << std::endl;
    std::cout << "Server: " << g_config.server.url << std::endl;
    std::cout << "======================================" << std::endl;
}

// ============================================================================
// 主函数
// ============================================================================
int main() {
    // 初始化随机数种子（用于生成蛇的颜色）
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // ==================== 加载配置文件 ====================

    LOG_INFO("Loading configuration from config.cfg...");

    if (!ConfigParser::load("config.cfg", g_config)) {
        LOG_ERROR("Failed to load config: " + ConfigParser::getError());
        std::cerr << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    // 应用配置到全局 config 命名空间
    g_config.apply();

    LOG_INFO("Configuration loaded successfully!");
    LOG_INFO("  Server: " + std::string(config::SERVER_URL));
    LOG_INFO("  Window: " + std::to_string(config::SCREEN_WIDTH) + "x" + std::to_string(config::SCREEN_HEIGHT));
    LOG_INFO("  Map: " + std::to_string(config::MAP_WIDTH) + "x" + std::to_string(config::MAP_HEIGHT));

    // 打印启动信息
    printStartupInfo();

    // ==================== 创建核心组件 ====================

    // 1. 状态管理器（所有组件共享）
    GameStateManager state;

    // 2. 网络管理器
    NetworkManager network(config::SERVER_URL, state);

    // 3. 渲染器
    Renderer renderer;

    // ==================== 初始化网络（登录 + 加入） ====================

    LOG_INFO("Initializing network connection...");

    if (!network.initialize(g_config.user.userId.c_str(), 
                            g_config.user.paste.c_str(), 
                            g_config.user.playerName.c_str())) {
        LOG_ERROR("Failed to initialize network. Please check your credentials.");
        std::cerr << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    LOG_INFO("Network initialization successful!");

    // ==================== 初始化渲染器 ====================

    LOG_INFO("Initializing renderer...");

    if (!renderer.initialize()) {
        LOG_ERROR("Failed to initialize renderer.");
        state.requestStop();
        return 1;
    }

    // ==================== 创建输入管理器 ====================

    InputManager input(state, renderer);

    // ==================== 启动网络线程 ====================

    LOG_INFO("Starting network thread...");

    std::thread networkThread(networkThreadFunc, &network);

    // ==================== 创建寻路线程 ====================

    std::thread pathfinderThread(pathPlanning, std::ref(state));

    // ==================== 主循环（渲染 + 输入） ====================

    LOG_INFO("Entering main loop...");

    while (!renderer.windowShouldClose() && !state.shouldStop()) {
        // 1. 处理输入
        input.update();

        // 2. 更新相机（跟随蛇头）
        renderer.updateCamera(state);

        // 3. 开始绘制
        renderer.beginDraw();

        // 4. 渲染游戏画面
        renderer.render(state);

        // 5. 结束绘制（显示到屏幕）
        renderer.endDraw();
    }

    // ==================== 清理资源 ====================

    LOG_INFO("Shutting down...");

    // 1. 请求线程停止
    state.requestStop();

    // 2. 等待线程结束
    if(networkThread.joinable()) {
        networkThread.join();
    }
    if(pathfinderThread.joinable()) {
        pathfinderThread.join();
    }

    // 3. 关闭渲染器
    renderer.shutdown();

    LOG_INFO("Goodbye!");

    return 0;
}
