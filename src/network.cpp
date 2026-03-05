/**
 * @file network.cpp
 * @brief 网络通信模块实现
 *
 * 实现与游戏服务器的所有通信逻辑
 *
 * 注意：此文件包含 CodingSnake.hpp，因此会引入 Windows API 依赖
 * 此文件不应与 raylib 相关代码混合
 */

#include <CodingSnake.hpp>
#include "network.hpp"
#include "pathfinder.hpp"

// ============================================================================
// NetworkManager 实现
// ============================================================================

NetworkManager::NetworkManager(const std::string& baseUrl, GameStateManager& state)
    : state_(state) {
    client_ = std::make_unique<CodingSnake>(baseUrl);
    client_->setVerbose(true);
    LOG_INFO("NetworkManager initialized with server: " << baseUrl);
}

NetworkManager::~NetworkManager() = default;

bool NetworkManager::initialize(const std::string& uid, const std::string& paste,
                                 const std::string& name) {
    uid_ = uid;
    paste_ = paste;
    name_ = name;

    bool flag = 1;
    while(flag) {
        try {
            client_->login(uid_, paste_);
            client_->join(name, "#ffafaf");
            flag = 0;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        }
    }

    state_.setAuthInfo(true);

    return 1;
}

std::string NetworkManager::loop(const GameState* game) {
    if(state_.shouldStop()) throw "stopping thread.";
    try {
        // ========== 1. 同步状态数据 ==========
        state_.updateRound(game->getCurrentRound());
        int mapWidth = 100, mapHeight = 100;

        // ========== 2. 解析数据 ==========
        std::vector<Snake> snakes = game->getAllPlayers();
        std::vector<Point> foods = game->getFoods();
        std::string playerId = game->getMySnake().id;

        state_.clearPendingMove();
        state_.setPlayerId(playerId);
        state_.updateGameState(snakes, foods, mapWidth, mapHeight);

        // ========== 3. 重新规划路径 ==========
        if(state_.isAutoNavigate()) {
            while(!state_.hasPendingMove())
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return state_.getPendingMove();
    } catch (const std::exception& e) {
        std::cerr << e.what();
    }
    return "right";
}

void NetworkManager::runLoop() {
    client_->run([&](const GameState& game) {
        return loop(&game);
    });
}

// ============================================================================
// 网络线程入口函数
// ============================================================================

void networkThreadFunc(NetworkManager* networkManager) {
    if(networkManager) {
        networkManager -> runLoop();
    }
}
