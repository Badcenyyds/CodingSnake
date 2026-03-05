# CodingSnake 手动游戏客户端

使用 raylib 渲染的贪吃蛇游戏手动控制客户端，采用模块化设计，支持双线程架构。

## 项目结构

```
SnakeGame/
├── include/                    # 头文件目录
│   ├── CodingSnake.hpp         # CodingSnake 官方库（HTTP 和 JSON 功能）
│   ├── common.hpp              # 公共类型定义、工具函数、配置常量
│   ├── config_parser.hpp       # 配置文件解析器
│   ├── input.hpp               # 输入处理模块接口
│   ├── network.hpp             # 网络通信模块接口
│   ├── pathfinder.hpp          # A* 路径规划模块
│   ├── renderer.hpp            # 渲染模块接口（raylib）
│   ├── state.hpp               # 游戏状态管理（线程安全）
│   ├── types.hpp               # 基础类型定义
│   └── windows_fix.hpp         # Windows 平台兼容性修复
├── src/                        # 源代码目录
│   ├── main.cpp                # 主程序入口
│   ├── common.cpp              # 公共函数实现
│   ├── config_parser.cpp       # 配置解析实现
│   ├── input.cpp               # 输入处理实现
│   ├── network.cpp             # 网络通信实现
│   ├── pathfinder.cpp          # 路径规划实现
│   └── renderer.cpp            # 渲染实现
├── raylib/                     # raylib 库
├── bin/                        # 编译输出目录
├── lib/                        # 库文件输出目录
├── obj/                        # 目标文件目录（传统 make）
├── build/                      # CMake 构建目录
├── config_example.cfg          # 配置文件示例
├── CMakeLists.txt              # CMake 构建配置
├── Makefile                    # 传统 Make 构建脚本
├── build.bat                   # Windows 快速构建脚本
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
| **common** | `common.hpp/cpp` | 公共类型（Point, Direction）、配置常量、工具函数 |
| **types** | `types.hpp` | 基础类型定义（Point, Rect 等） |
| **state** | `state.hpp` | 线程安全的游戏状态管理，使用读写锁保护数据 |
| **network** | `network.hpp/cpp` | HTTP 通信、登录认证、状态轮询、移动提交 |
| **renderer** | `renderer.hpp/cpp` | raylib 窗口管理、游戏世界渲染、UI 绘制 |
| **input** | `input.hpp/cpp` | 键盘/鼠标输入处理 |
| **config_parser** | `config_parser.hpp/cpp` | INI 格式配置文件解析 |
| **pathfinder** | `pathfinder.hpp/cpp` | A* 路径规划算法（可选功能） |

## 快速开始

### 1. 配置认证信息

复制配置文件示例并编辑：

```bash
copy config_example.cfg config.cfg
```

编辑 `config.cfg`，修改以下配置：

```ini
[user]
user_id = 你的用户 ID
paste = 你的认证 Paste
player_name = 你的蛇名字

[server]
url = http://csapi.seveoi.icu:18080
```

### 2. 编译

使用批处理脚本。

```batch
build.bat
```

### 3. 控制说明

| 按键 | 功能 |
|------|------|
| **W / ↑** | 向上移动 |
| **A / ←** | 向左移动 |
| **S / ↓** | 向下移动 |
| **D / →** | 向右移动 |
| **N** | 启动自动导航 |
| **滚轮** | 缩放视图 |
| **F3** | 切换调试信息 |

### 4. 运行

编译产物在 \bin 目录下

```bash
.\bin\snake_game.exe
```

## 详细使用说明

### 移动机制

1. **手动移动**：使用 WASD 或方向键选择你想要的移动方向
3. **自动导航**：按 [N] 键可以启动自动导航功能

### 游戏流程

```
1. 程序启动
   │
   ▼
2. 读取配置文件（config.cfg）
   │
   ▼
3. 网络初始化（登录 → 加入游戏）
   │
   ▼
4. 启动网络线程（开始轮询服务器）
   │
   ▼
5. 进入主循环
   ├── 处理输入
   ├── 更新相机
   ├── 渲染画面
   └── 等待 vsync
   │
   ▼
6. 关闭窗口或游戏结束
   │
   ▼
7. 清理资源，退出
```

## 配置选项

### 用户认证 `[user]`

| 选项 | 说明 |
|------|------|
| `user_id` | 用户 ID |
| `paste` | 认证 Paste |
| `player_name` | 蛇的名字 |

### 服务器 `[server]`

| 选项 | 说明 |
|------|------|
| `url` | 服务器 API 地址 |

### 窗口 `[window]`

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `width` | 1280 | 窗口宽度 |
| `height` | 720 | 窗口高度 |
| `title` | CodingSnake - Manual Play | 窗口标题 |

### 渲染 `[render]`

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `base_tile_size` | 1.0 | 基础网格大小 |
| `min_zoom` | 3.0 | 最小缩放 |
| `max_zoom` | 100.0 | 最大缩放 |
| `zoom_speed` | 0.05 | 缩放速度 |
| `camera_lerp_speed` | 5.0 | 相机平滑速度 |
| `target_fps` | 60 | 目标帧率 |

### 游戏 `[game]`

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `map_width` | 100 | 地图宽度 |
| `map_height` | 100 | 地图高度 |
| `network_poll_interval_ms` | 250 | 网络轮询间隔（毫秒） |

### 添加新功能

1. **新输入**：在 `src/input.cpp` 的 `handleActionKeys()` 中添加
2. **新渲染**：在 `src/renderer.cpp` 中添加新的绘制函数
3. **新状态**：在 `include/state.hpp` 中添加数据成员和访问方法
4. **新网络功能**：在 `src/network.cpp` 中添加 API 调用

## 依赖项

| 依赖 | 用途 | 来源 |
|------|------|------|
| **raylib 5.5** | 图形渲染 | `raylib/` 目录 |
| **nlohmann/json** | JSON 解析 | 通过 `CodingSnake.hpp` 引入 |
| **cpp-httplib** | HTTP 客户端 | 通过 `CodingSnake.hpp` 引入 |
| **MinGW-w64** | C++ 编译器 | 系统安装 |
| **CMake 3.16+** | 构建系统（可选） | 系统安装 |

## 编译选项详解

### 传统 Make

```makefile
# 可修改的选项
CXXSTD = -std=c++17          # C++ 标准
CXX = g++                     # 编译器
CXXFLAGS = -O2                # 优化级别
WARNINGS = -Wall -Wextra      # 警告级别
```

### CMake

```bash
# 构建类型
cmake -B build -DCMAKE_BUILD_TYPE=Release   # Release
cmake -B build -DCMAKE_BUILD_TYPE=Debug     # Debug

# 并行编译
cmake --build build --parallel 8
```

## 故障排除

### 问题：编译时找不到 raylib

**解决**：检查 `Makefile` 或 `CMakeLists.txt` 中的 `RAYLIB_DIR` 路径是否正确

### 问题：链接错误 `undefined reference to ws2_32`

**解决**：确保 `Makefile` 或 `CMakeLists.txt` 中包含 `-lws2_32`

### 问题：游戏运行时网络请求失败

**解决**：
1. 去网址首页观察这个游戏是否还在提供服务
2. 检查服务器地址是否正确
2. 确认你的 `user_id` 和 `paste` 有效
3. 检查网络连接

### 问题：配置文件未找到

**解决**：确保 `config.cfg` 存在于可执行文件同一目录下，或从 `config_example.cfg` 复制一份

## 构建脚本说明

### Makefile 目标

| 目标 | 说明 |
|------|------|
| `all` | 使用传统 make 编译 |
| `cmake` | 使用 CMake 编译（Release） |
| `cmake-debug` | 使用 CMake 编译（Debug） |
| `clean` | 清理传统 make 产物 |
| `cmake-clean` | 清理 CMake 产物 |
| `clean-all` | 清理所有产物 |
| `run` | 编译并运行（传统 make） |
| `run-cmake` | 运行 CMake 编译的版本 |
| `debug` | 编译调试版本 |
| `help` | 显示帮助信息 |

## 许可证

本项目基于 [CodingSnake](https://github.com/seve42/CodingSnake) 开发。

raylib 使用 [Zlib 许可证](https://github.com/raysan5/raylib/blob/master/LICENSE)。