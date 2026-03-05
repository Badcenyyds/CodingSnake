# CodingSnake 手动游戏客户端

使用 raylib 渲染的贪吃蛇游戏手动控制客户端，采用模块化设计，支持双线程架构。

## 项目结构

```
SnakeGame/
├── include/                    # 头文件目录
│   ├── common.hpp              # 公共类型定义、工具函数、配置常量
│   ├── state.hpp               # 游戏状态管理（线程安全）
│   ├── network.hpp             # 网络通信模块接口
│   ├── renderer.hpp            # 渲染模块接口（raylib）
│   └── input.hpp               # 输入处理模块接口
├── src/                        # 源代码目录
│   ├── main.cpp                # 主程序入口
│   ├── network.cpp             # 网络通信实现
│   ├── renderer.cpp            # 渲染实现
│   └── input.cpp               # 输入处理实现
├── raylib/                     # raylib 库（已包含）
├── CodingSnake.hpp             # CodingSnake 官方库（提供 HTTP 和 JSON 功能）
├── Makefile                    # 构建脚本
└── README.md                   # 本文件
```

## 架构设计

### 双线程模型

```
┌─────────────────────────────────────────────────────────┐
│                     主线程                               │
│  ┌─────────────┐    ┌─────────────┐                     │
│  │  Renderer   │◄──►│   Input     │                     │
│  │  (渲染)     │    │  (输入处理)  │                     │
│  └─────────────┘    └─────────────┘                     │
│         ▲                   ▲                            │
│         │                   │                            │
│         └─────────┬─────────┘                            │
│                   │                                      │
│         ┌─────────▼─────────┐                            │
│         │ GameStateManager  │                            │
│         │   (线程安全数据)   │                            │
│         └─────────▲─────────┘                            │
│                   │                                      │
└───────────────────│──────────────────────────────────────┘
                    │
┌───────────────────▼──────────────────────────────────────┐
│                    网络线程                               │
│         ┌─────────────────────┐                          │
│         │   NetworkManager    │                          │
│         │    (通信/轮询)      │                          │
│         └─────────────────────┘                          │
└──────────────────────────────────────────────────────────┘
```

### 模块职责

| 模块 | 文件 | 职责 |
|------|------|------|
| **common** | `common.hpp` | 公共类型（Point, Snake）、配置常量、工具函数 |
| **state** | `state.hpp` | 线程安全的游戏状态管理，使用读写锁保护数据 |
| **network** | `network.hpp/cpp` | HTTP 通信、登录认证、状态轮询、移动提交 |
| **renderer** | `renderer.hpp/cpp` | raylib 窗口管理、游戏世界渲染、UI 绘制 |
| **input** | `input.hpp/cpp` | 键盘/鼠标输入处理 |

## 快速开始

### 1. 配置认证信息

编辑 `src/main.cpp`，修改以下常量：

```cpp
constexpr const char* USER_ID = "你的用户 ID";
constexpr const char* PASTE = "你的认证 Paste";
constexpr const char* PLAYER_NAME = "你的蛇名字";
```

### 2. 编译

确保你已经安装了：
- MinGW-w64（g++ 编译器）
- raylib 5.5（已包含在 `raylib/` 目录）

```bash
# 编译
make

# 或者编译并运行
make run

# 清理编译产物
make clean

# 编译调试版本
make debug
```

### 3. 控制说明

| 按键 | 功能 |
|------|------|
| **W / ↑** | 向上移动 |
| **A / ←** | 向左移动 |
| **S / ↓** | 向下移动 |
| **D / →** | 向右移动 |
| **空格** | 锁定/解锁移动 |
| **Enter** | 立即提交移动 |
| **滚轮** | 缩放视图 |
| **F3** | 切换调试信息 |

### 4. 运行

```bash
./snake_game.exe
```

## 详细使用说明

### 移动机制

1. **选择方向**：使用 WASD 或方向键选择你想要的移动方向
2. **锁定移动**：按空格键锁定方向，锁定后会在下一回合自动提交
3. **立即提交**：按 Enter 键可以立即发送移动指令到服务器

### 游戏流程

```
1. 程序启动
   │
   ▼
2. 网络初始化（登录 → 加入游戏）
   │
   ▼
3. 启动网络线程（开始轮询服务器）
   │
   ▼
4. 进入主循环
   ├── 处理输入
   ├── 更新相机
   ├── 渲染画面
   └── 等待 vsync
   │
   ▼
5. 关闭窗口或游戏结束
   │
   ▼
6. 清理资源，退出
```

## 修改与扩展

### 修改配置

所有配置常量都在 `include/common.hpp` 中：

```cpp
namespace config {
    constexpr const char* SERVER_URL = "http://csapi.seveoi.icu:18080";
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;
    constexpr float BASE_TILE_SIZE = 20.0f;
    constexpr int NETWORK_POLL_INTERVAL_MS = 200;
    // ... 更多配置
}
```

### 添加新功能

1. **新输入**：在 `src/input.cpp` 的 `handleActionKeys()` 中添加
2. **新渲染**：在 `src/renderer.cpp` 中添加新的绘制函数
3. **新状态**：在 `include/state.hpp` 中添加数据成员和访问方法
4. **新网络功能**：在 `src/network.cpp` 中添加 API 调用

### 示例：添加暂停功能

1. 在 `state.hpp` 中添加暂停状态：
```cpp
bool isPaused() const { return isPaused_; }
void setPaused(bool paused) { isPaused_ = paused; }
private: bool isPaused_ = false;
```

2. 在 `input.cpp` 中添加暂停键：
```cpp
if (IsKeyPressed(KEY_P)) {
    state_.setPaused(!state_.isPaused());
}
```

3. 在 `renderer.cpp` 中绘制暂停提示：
```cpp
if (state.isPaused()) {
    DrawText("PAUSED", screenWidth/2 - 50, screenHeight/2, 30, WHITE);
}
```

## 依赖项

| 依赖 | 用途 | 来源 |
|------|------|------|
| **raylib 5.5** | 图形渲染 | `raylib/` 目录 |
| **nlohmann/json** | JSON 解析 | 通过 `CodingSnake.hpp` 引入 |
| **cpp-httplib** | HTTP 客户端 | 通过 `CodingSnake.hpp` 引入 |
| **MinGW-w64** | C++ 编译器 | 系统安装 |

## 编译选项详解

```makefile
# 在 Makefile 中可修改的选项
CXXSTD = -std=c++17          # C++ 标准
CXX = g++                     # 编译器
CXXFLAGS = -O2                # 优化级别
WARNINGS = -Wall -Wextra      # 警告级别
```

## 故障排除

### 问题：编译时找不到 raylib

**解决**：检查 `Makefile` 中的 `RAYLIB_DIR` 路径是否正确

### 问题：链接错误 `undefined reference to ws2_32`

**解决**：确保 `Makefile` 中包含 `-lws2_32`

### 问题：游戏运行时网络请求失败

**解决**：
1. 检查服务器地址是否正确
2. 确认你的 `uid` 和 `paste` 有效
3. 检查网络连接

## 许可证

本项目基于 [CodingSnake](https://github.com/seve42/CodingSnake) 开发。

raylib 使用 [Zlib 许可证](https://github.com/raysan5/raylib/blob/master/LICENSE)。
