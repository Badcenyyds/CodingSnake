/**
 * @file config_parser.cpp
 * @brief INI 风格配置文件解析器实现
 */

#include "config_parser.hpp"
#include "common.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

// ============================================================================
// 静态成员初始化
// ============================================================================
std::string ConfigParser::lastError_;

// ============================================================================
// Config 默认构造函数 - 使用 common.hpp 中的默认值
// ============================================================================
Config::Config() {
    // 用户认证（空默认值，必须从配置文件读取）
    user.userId = "";
    user.paste = "";
    user.playerName = "";

    // 服务器
    server.url = config::SERVER_URL;

    // 窗口
    window.width = config::SCREEN_WIDTH;
    window.height = config::SCREEN_HEIGHT;
    window.title = config::WINDOW_TITLE;

    // 渲染
    render.baseTileSize = config::BASE_TILE_SIZE;
    render.minZoom = config::MIN_ZOOM;
    render.maxZoom = config::MAX_ZOOM;
    render.zoomSpeed = config::ZOOM_SPEED;
    render.cameraLerpSpeed = config::CAMERA_LERP_SPEED;
    render.targetFps = config::TARGET_FPS;

    // 游戏
    game.mapWidth = config::MAP_WIDTH;
    game.mapHeight = config::MAP_HEIGHT;
    game.networkPollIntervalMs = config::NETWORK_POLL_INTERVAL_MS;
}

void Config::apply() const {
    // 服务器
    config::SERVER_URL = server.url.c_str();

    // 窗口
    config::SCREEN_WIDTH = window.width;
    config::SCREEN_HEIGHT = window.height;
    config::WINDOW_TITLE = window.title.c_str();

    // 渲染
    config::BASE_TILE_SIZE = render.baseTileSize;
    config::MIN_ZOOM = render.minZoom;
    config::MAX_ZOOM = render.maxZoom;
    config::ZOOM_SPEED = render.zoomSpeed;
    config::CAMERA_LERP_SPEED = render.cameraLerpSpeed;
    config::TARGET_FPS = render.targetFps;

    // 游戏
    config::MAP_WIDTH = game.mapWidth;
    config::MAP_HEIGHT = game.mapHeight;
    config::NETWORK_POLL_INTERVAL_MS = game.networkPollIntervalMs;
}

// ============================================================================
// ConfigParser 实现
// ============================================================================

std::string ConfigParser::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool ConfigParser::isCommentOrEmpty(const std::string& line) {
    std::string trimmed = trim(line);
    return trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';';
}

std::optional<std::string> ConfigParser::parseSection(const std::string& line) {
    std::string trimmed = trim(line);
    if (trimmed.size() < 2 || trimmed.front() != '[' || trimmed.back() != ']') {
        return std::nullopt;
    }
    return trimmed.substr(1, trimmed.size() - 2);
}

std::optional<std::pair<std::string, std::string>> ConfigParser::parseKeyValue(const std::string& line) {
    std::string trimmed = trim(line);
    size_t eqPos = trimmed.find('=');
    if (eqPos == std::string::npos) {
        return std::nullopt;
    }

    std::string key = trim(trimmed.substr(0, eqPos));
    std::string value = trim(trimmed.substr(eqPos + 1));

    if (key.empty()) {
        return std::nullopt;
    }

    return std::make_pair(key, value);
}

bool ConfigParser::load(const std::string& filename, Config& config) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        lastError_ = "无法打开配置文件：" + filename;
        return false;
    }

    std::string line;
    std::string currentSection;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;

        // 跳过注释和空行
        if (isCommentOrEmpty(line)) {
            continue;
        }

        // 尝试解析节标记
        auto section = parseSection(line);
        if (section.has_value()) {
            currentSection = section.value();
            continue;
        }

        // 尝试解析键值对
        auto kv = parseKeyValue(line);
        if (!kv.has_value()) {
            lastError_ = "第 " + std::to_string(lineNum) + " 行格式错误：" + line;
            return false;
        }

        const std::string& key = kv->first;
        const std::string& value = kv->second;

        // 根据当前节和键名设置配置值
        if (currentSection == "user") {
            if (key == "user_id") config.user.userId = value;
            else if (key == "paste") config.user.paste = value;
            else if (key == "player_name") config.user.playerName = value;
        }
        else if (currentSection == "server") {
            if (key == "url") config.server.url = value;
        }
        else if (currentSection == "window") {
            if (key == "width") config.window.width = std::stoi(value);
            else if (key == "height") config.window.height = std::stoi(value);
            else if (key == "title") config.window.title = value;
        }
        else if (currentSection == "render") {
            if (key == "base_tile_size") config.render.baseTileSize = std::stof(value);
            else if (key == "min_zoom") config.render.minZoom = std::stof(value);
            else if (key == "max_zoom") config.render.maxZoom = std::stof(value);
            else if (key == "zoom_speed") config.render.zoomSpeed = std::stof(value);
            else if (key == "camera_lerp_speed") config.render.cameraLerpSpeed = std::stof(value);
            else if (key == "target_fps") config.render.targetFps = std::stoi(value);
        }
        else if (currentSection == "game") {
            if (key == "map_width") config.game.mapWidth = std::stoi(value);
            else if (key == "map_height") config.game.mapHeight = std::stoi(value);
            else if (key == "network_poll_interval_ms") config.game.networkPollIntervalMs = std::stoi(value);
        }
    }

    // 验证必填字段
    if (config.user.userId.empty()) {
        lastError_ = "配置缺少必填项：[user] user_id";
        return false;
    }
    if (config.user.paste.empty()) {
        lastError_ = "配置缺少必填项：[user] paste";
        return false;
    }

    return true;
}

const std::string& ConfigParser::getError() {
    return lastError_;
}
