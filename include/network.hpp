/**
 * @file network.hpp
 * @brief 网络通信模块 - 负责与游戏服务器交互
 *
 * 功能：
 * - HTTP 客户端封装
 * - 登录/加入游戏
 * - 轮询游戏状态
 * - 发送移动指令
 *
 * 运行在独立的网络线程中，定期轮询服务器更新游戏状态
 *
 * 注意：此头文件不直接包含 CodingSnake.hpp，依赖被封装在 .cpp 文件中
 */

#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "state.hpp"
// #include "CodingSnake.hpp"
#include <memory>

#ifndef CODINGSNAKE_HPP
struct CodingSnake;
struct GameState;
#endif

/**
 * @brief 网络通信管理器
 *
 * 封装所有与服务器的通信逻辑，包括：
 * - 连接管理
 * - 认证（登录/加入）
 * - 状态轮询
 * - 移动提交
 */

class NetworkManager {
private:
    std::unique_ptr<CodingSnake> client_;  // CodingSnake 客户端
    GameStateManager& state_;                   // 状态管理器（引用）

    std::string uid_;      // 用户 ID
    std::string paste_;    // 认证 paste
    std::string name_;     // 玩家名字

public:
    /**
     * @brief 构造函数
     *
     * @param baseUrl 服务器地址
     * @param state 状态管理器引用
     */
    NetworkManager(const std::string& baseUrl, GameStateManager& state);

    ~NetworkManager();

    /**
     * @brief 初始化网络连接（登录 + 加入）
     *
     * @param uid 用户 ID
     * @param paste 认证 paste
     * @param name 玩家名字
     * @return true 成功，false 失败
     */
    bool initialize(const std::string& uid, const std::string& paste,
                    const std::string& name);

    /**
     * @brief 网络循环（在网络线程中运行）
     *
     * 同步最新状态，并发送待处理的移动指令
     */
    std::string loop(const GameState* game);

    void runLoop();
};

/**
 * @brief 网络线程入口函数
 *
 * @param networkManager 网络管理器指针
 */
void networkThreadFunc(NetworkManager* networkManager);

#endif // NETWORK_HPP
