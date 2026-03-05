/**
 * @file config_parser.hpp
 * @brief 简单的 INI 风格配置文件解析器
 *
 * 支持格式：
 * - [section] 节标记
 * - key = value 键值对
 * - # 或 ; 开头的注释行
 * - 空行
 */

#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <string>
#include <map>
#include <optional>

/**
 * @brief 配置解析器类
 *
 * 使用方法：
 * @code
 * Config config;
 * if (!ConfigParser::load("config.cfg", config)) {
 *     // 处理错误
 * }
 * @endcode
 */
class ConfigParser {
public:
    /**
     * @brief 从文件加载配置
     * @param filename 配置文件路径
     * @param config 输出配置对象
     * @return 成功返回 true，失败返回 false 并设置错误信息
     */
    static bool load(const std::string& filename, class Config& config);

    /**
     * @brief 获取最后错误信息
     */
    static const std::string& getError();

private:
    static std::string lastError_;

    /**
     * @brief 修剪字符串两端空白
     */
    static std::string trim(const std::string& str);

    /**
     * @brief 检查是否为注释或空行
     */
    static bool isCommentOrEmpty(const std::string& line);

    /**
     * @brief 解析节标记 [section]
     * @return 节名称，如果不是节标记则返回空
     */
    static std::optional<std::string> parseSection(const std::string& line);

    /**
     * @brief 解析键值对 key = value
     * @return pair<key, value>，如果不是键值对则返回 nullopt
     */
    static std::optional<std::pair<std::string, std::string>> parseKeyValue(const std::string& line);
};

/**
 * @brief 配置数据结构
 *
 * 包含所有可从配置文件读取的参数
 */
struct Config {
    // ========== 用户认证配置 ==========
    struct User {
        std::string userId;
        std::string paste;
        std::string playerName;
    } user;

    // ========== 服务器配置 ==========
    struct Server {
        std::string url;
    } server;

    // ========== 窗口配置 ==========
    struct Window {
        int width;
        int height;
        std::string title;
    } window;

    // ========== 渲染配置 ==========
    struct Render {
        float baseTileSize;
        float minZoom;
        float maxZoom;
        float zoomSpeed;
        float cameraLerpSpeed;
        int targetFps;
    } render;

    // ========== 游戏配置 ==========
    struct Game {
        int mapWidth;
        int mapHeight;
        int networkPollIntervalMs;
    } game;

    /**
     * @brief 使用默认值初始化配置
     */
    Config();

    /**
     * @brief 应用到全局配置（更新 config 命名空间）
     */
    void apply() const;
};

#endif // CONFIG_PARSER_HPP
