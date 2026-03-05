/**
 * @file pathfinder.cpp
 * @brief A* 路径规划模块 - 带权重避障的蛇路径搜索（实现）
 */

#include "pathfinder.hpp"
#include "state.hpp"

// ==================== 预计算权重 ====================

std::vector<std::vector<int>> precomputeDistances(
    const std::vector<Snake>& snakes,
    const std::string& myId, int mode) {

    int mapWidth_ = config::MAP_WIDTH, mapHeight_ = config::MAP_HEIGHT;

    // 记录每个位置到最近敌方蛇头的距离
    std::vector<std::vector<int>> dist(
        mapWidth_, std::vector<int>(mapHeight_, -1));
    std::queue<Point> q;
    Point myHead;

    for(const auto& snake : snakes) {
        if (snake.id == myId && !snake.blocks.empty()) {
            myHead = snake.head;
            break;
        }
    }

    if(mode == 2) {
        // 加入边界
        for(int i = 0; i < mapWidth_; ++i) {
            dist[i][0] = 0, dist[i][mapHeight_ - 1] = 0;
            dist[0][i] = 0, dist[mapWidth_ - 1][i] = 0;
            q.push({i, 0}), q.push({i, mapHeight_ - 1});
            q.push({0, i}), q.push({mapWidth_ - 1, i});
        }
    }

    for (const auto& snake : snakes) {
        if (snake.blocks.empty()) continue;

        bool isMySnake = (snake.id == myId);

        // ========== 第一遍：标记蛇身碰撞位置 ==========
        for (size_t i = 0; i < snake.blocks.size(); ++i) {
            const auto& block = snake.blocks[i];
            int timeleft = static_cast<int>(snake.blocks.size() - i);
            if(block.distance(myHead) >= timeleft + 2) {
                continue; // 这个块在抵达会消失，不标记为碰撞
            }
            dist[block.x][block.y] = 0;
            if(mode == 2) q.push(block);
        }

        // ========== 第二遍：处理蛇头 ==========
        if ((mode == 1) ^ isMySnake) continue;

        const auto& head = snake.head;
        // BFS 计算从该蛇头到所有位置的距离
        dist[head.x][head.y] = 0;
        q.push(head);
    }

    while (!q.empty()) {
        Point cur = q.front();
        q.pop();

        int curDist = dist[cur.x][cur.y];

        // 四个方向
        static const int dx[] = {0, 0, -1, 1};
        static const int dy[] = {-1, 1, 0, 0};

        for (int d = 0; d < 4; ++d) {
            int nx = cur.x + dx[d];
            int ny = cur.y + dy[d];

            if (nx >= 0 && nx < mapWidth_ &&
                ny >= 0 && ny < mapHeight_ &&
                dist[nx][ny] == -1) {
                dist[nx][ny] = curDist + 1;
                q.push({nx, ny});
            }
        }
    }
    
    return dist;
}

std::vector<std::vector<float>> precomputeWeights(
    const std::vector<Snake>& snakes,
    const std::string& myId) {

    int mapWidth_ = config::MAP_WIDTH, mapHeight_ = config::MAP_HEIGHT;

    // 初始化权重网格，默认安全权重
    std::vector<std::vector<float>> weights(
        mapWidth_, std::vector<float>(mapHeight_, 0));

    auto dist = precomputeDistances(snakes, myId);

    for (int i = 0; i < mapWidth_; ++i) {
        for (int j = 0; j < mapHeight_; ++j) {
            if (dist[i][j] <= 1) {
                weights[i][j] = WEIGHT_COLLISION;  // 碰撞位置
            } else if (dist[i][j] > 0) {
                // weights[i][j] = std::max(0.0f, WEIGHT_COLLISION - dist[i][j] * 10);
            }
        }
    }

    return weights;
}

// ==================== 辅助函数 ====================

float getCellWeight(const std::vector<std::vector<float>>& weights,
                              const Point& pos) {
    if (pos.x < 0 || pos.x >= config::MAP_WIDTH || pos.y < 0 || pos.y >= config::MAP_HEIGHT) {
        return WEIGHT_COLLISION;  // 墙壁
    }
    return weights[pos.x][pos.y];
}

std::vector<Point> getNeighbors(const Point& pos) {
    return {
        Point(pos.x, pos.y - 1),  // 上
        Point(pos.x, pos.y + 1),  // 下
        Point(pos.x - 1, pos.y),  // 左
        Point(pos.x + 1, pos.y)   // 右
    };
}

bool isValid(const Point& pos) {
    return pos.x >= 0 && pos.x < config::MAP_WIDTH && pos.y >= 0 && pos.y < config::MAP_HEIGHT;
}

int makeKey(const Point& pos) {
    return static_cast<int>(pos.x) * config::MAP_HEIGHT + pos.y;
}

// ==================== 主路径搜索函数 ====================

// 参与考虑的食物数量
static constexpr int K_FOODS = 24;
// 路径规划中考虑的步数
static constexpr int K_STEPS = 64;
// A* 中每一步产生的代价
const float STEPWEIGHT = 1;

void pathPlanning(GameStateManager& state) {
    while(!state.hasValidState())
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

    size_t k = K_FOODS;

    std::vector<Snake> snakes;
    std::vector<Point> foods;
    std::string myId;
    Point startPos;

    std::vector<std::vector<float>> weights;
    // 依次为 到最近敌方蛇头的距离，到自己蛇头的距离，离边界的距离
    std::vector<std::vector<int>> distOt, distMe, distBo;

    std::vector<std::pair<int, size_t>> foodDistances; 
    // 将起点和前K近的途径点存起来
    std::vector<Point> points;
    // 存储我到该食物的距离-其他蛇到该食物的距离。用来考虑被抢走的情况。
    std::vector<int> capturev;
    std::vector<std::vector<std::pair<float, std::vector<Point>>>> paths;

    auto updatePara = [&]() {
        foods = state.getFoods();
        snakes = state.getSnakes();
        myId = state.getPlayerId();
        startPos = state.getMySnake().head;
        weights = precomputeWeights(snakes, myId);
        distOt = precomputeDistances(snakes, myId);
        distMe = precomputeDistances(snakes, myId, 1);
        // distBo = precomputeDistances(snakes, myId, 2);

        foodDistances.clear();
        for (size_t i = 0; i < foods.size(); ++i) {
            int dis = distMe[foods[i].x][foods[i].y] - distOt[foods[i].x][foods[i].y];
            foodDistances.emplace_back(dis, i);
        }
        // 按距离排序
        std::sort(foodDistances.begin(), foodDistances.end(),
            [](const auto& a, const auto& b) { return a.first < b.first; });

        points.clear(), capturev.clear();
        points.push_back(startPos), capturev.push_back(-1);
        for (size_t i = 0; i < k; ++i) {
            points.push_back(foods[foodDistances[i].second]);
            capturev.push_back(foodDistances[i].first);
        }

        paths.resize(k+1, std::vector<std::pair<float, std::vector<Point>>>(k+1));
        for (size_t i = 0; i <= k; ++i) {
            for (size_t j = i+1; j <= k; ++j) {
                auto [cost, path] = findPath(points[i], points[j], weights);
                paths[i][j].second = paths[j][i].second = path;
                if (!path.empty()) {
                    paths[i][j].first = cost;
                } else {
                    paths[i][j].first = WEIGHT_COLLISION;  // 无法到达
                }
                paths[j][i].first = paths[i][j].first;
            }
        }

    };

    std::vector<Point> path(K_FOODS+1);
    static int lastRound = -1;
    
    while(!state.shouldStop()) {
        if(!state.isAutoNavigate()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if(!state.isUpdated(lastRound)) {
            continue;
        }
        updatePara();

        // auto start_time = std::chrono::high_resolution_clock::now();
        // int iter_count = 0;

        // ========== 2. 对所有食物运行退火 ==========

        std::vector<int> permu(k+1);
        for (size_t i = 0; i <= k; ++i) permu[i] = i;

        float best = -1e9; size_t len = 0;
        std::vector<int> permuBest = permu;
        // 估价函数：
        auto calc = [&]() {
            float distance = 0, score = 0; int cnt = 0;
            cnt = 1;
            score = -paths[permu[0]][permu[1]].first;
            // for (size_t i = 1; i <= K_FOODS; ++i) {
                // float cost = paths[permu[i-1]][permu[i]].first;

                // distance += cost;
                // score += 1.f + 1.f / distance;

                // if(distance > K_STEPS) break;

                // ++cnt;
            // }
            if(score > best) {
                best = score;
                len = cnt;
                permuBest = permu;
            }
            return score;
        };

        // 引用全局随机数引擎
        auto& rng = RndNumGen::rng;
        float now = calc();
        
        // 退火优化路径顺序
        // 注意：k >= 2，所以有效索引范围是 [0, k-1]
        // 我们只交换 [1, k-1] 范围内的食物点（保持起点 permu[0] 不变）
        for (double T = 1e3; T > 1e-3; T *= 0.9998) {
            int x = rng() % (k - 1) + 1, y = rng() % (k - 1) + 2;
            while(std::abs(x - y) <= 1) {
                x = rng() % (k - 1) + 1;
                y = rng() % (k - 1) + 2;
            }

            std::reverse(permu.begin() + x, permu.begin() + y);  // 反转 [x, y-1] 范围内的食物顺序
            float ans = calc();

            // 避免 exp 溢出：限制 (now - ans) / T 的范围
            float diff = (ans - now) / T;
            bool accept = false;
            if (ans > now) {
                accept = true;
            } else if (diff > -20) { 
                accept = std::exp(diff) > (rng() / static_cast<float>(rng.max()));
            }

            if (accept) {
                now = ans;
            } else {
                std::reverse(permu.begin() + x, permu.begin() + y);  // 恢复原顺序
            }
            // ++iter_count;
        }

        std::cerr << best << ' ' << len << "\n";
        permu = permuBest;
        for(size_t i = 0; i <= k; ++i) {
            path[i] = points[permu[i]];
        }

        // 构建路径
        std::vector<Point> bestPath;
        for (size_t i = 1; i <= len; ++i) {
            const auto& segment = paths[permu[i-1]][permu[i]].second;
            if (bestPath.empty()) {
                bestPath = segment;
            } else {
                bestPath.insert(bestPath.end(), segment.begin(), segment.end());
            }
        }

        // auto end_time = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double> elapsed = end_time - start_time;
        // std::cerr << "time: " << elapsed.count() << "s\n";

        // ========== 3. Fallback：如果找不到完整路径，尝试找到一个安全的下一步 ==========
        if (bestPath.empty()) {
            bestPath = findSafeNextStep(startPos, weights);
        }

        // std::cerr << "Planned path length = " << bestPath.size() << ", best score = " << best << "\n";
        state.setPlannedPath(bestPath);
        state.setPendingMove(getNextMove(startPos, bestPath));
    }
}

// ==================== A* 搜索实现 ====================

/**
 * @brief A* 节点
 */
struct Node {
    Point pos;
    float hCost;  // 从当前节点到目标的启发式代价
    float weightCost;  // 累积的权重代价（越小越好）
    int parentKey;  // 父节点的 key，-1 表示没有父节点

    Node() :  hCost(0), weightCost(0), parentKey(-1) {}
    Node(const Point& p, float h, float w, int pKey = -1)
        : pos(p), hCost(h), weightCost(w), parentKey(pKey) {}

    // 综合代价：权重优先，距离其次
    float totalCost() const {
        return weightCost + STEPWEIGHT * hCost; 
    }

    // 优先队列比较：综合代价小的优先
    inline bool operator<(const Node& other) const {
        if(totalCost() == other.totalCost()) {
            return hCost > other.hCost;
        }
        return totalCost() > other.totalCost();  // 小根堆
    }
};

std::pair<float, std::vector<Point>> findPath(
    const Point& start,
    const Point& target,
    const std::vector<std::vector<float>>& weights) {

    // 快速路径：起点等于终点
    if (start == target) {
        return {0.0f, {start}};
    }

    // 优先队列
    std::priority_queue<Node> openSet;
    // 位置到父节点的映射（用于重建路径）
    std::unordered_map<int, int> cameFrom;
    // 记录到达每个位置的最佳 gCost (weightCost)
    std::unordered_map<int, float> gCost;
    // 记录已关闭的节点（已处理过的最优节点）
    std::unordered_map<int, float> closedSet;

    // 创建起始节点
    float h0 = start.distance(target);
    int startKey = makeKey(start);

    Node startNode(start, h0, 0, -1);
    openSet.push(startNode);
    gCost[startKey] = 0;
    cameFrom[startKey] = -1;

    int iter = 0;
    static const int MAX_ITER = 4096;  // 提高迭代上限
    static int aStarCount = 0, maxIterCount = 0;

    while (!openSet.empty()) {
        Node current = openSet.top(); openSet.pop();
        int currentKey = makeKey(current.pos);

        // 检查是否已在 closedSet 中（已处理过的更优解）
        auto closedIt = closedSet.find(currentKey);
        if (closedIt != closedSet.end()) {
            if (current.weightCost >= closedIt->second) {
                continue;
            }
        }

        // 跳过已处理的更优节点
        auto it = gCost.find(currentKey);
        if (it != gCost.end()) {
            if (current.weightCost > it->second) {
                continue;
            }
        }

        // 到达目标
        if (current.pos == target) {
            maxIterCount = std::max(maxIterCount, iter);
            if(++aStarCount > 1024) {
                std::cerr << "maxIterCount of last 1024 times: " << maxIterCount << "\n";
                aStarCount = 0, maxIterCount = 0;
            }
            // 重建路径：从 target 回溯到 start
            std::vector<Point> path;
            int key = currentKey;
            while (key != startKey && key != -1) {
                // 从 key 反推位置
                Point p;
                p.y = key % config::MAP_HEIGHT;
                p.x = (key - p.y) / config::MAP_HEIGHT;
                path.push_back(p);
                key = cameFrom[key];
            }
            std::reverse(path.begin(), path.end());
            return {current.totalCost(), path};
        }

        // 将当前节点加入 closedSet
        closedSet[currentKey] = current.weightCost;

        if(++iter > MAX_ITER) {
            // std::cerr << "A* timeout (iter=" << iter << ").\n";
            break;
        }

        // 探索邻居
        for (const auto& neighbor : getNeighbors(current.pos)) {
            if (!isValid(neighbor)) continue;

            int neighborKey = makeKey(neighbor);

            // 跳过已在 closedSet 中的节点
            if (closedSet.count(neighborKey)) continue;

            float cellWeight = getCellWeight(weights, neighbor);

            // 跳过碰撞位置
            if (cellWeight == WEIGHT_COLLISION) continue;

            float newWeightCost = current.weightCost + cellWeight + STEPWEIGHT;

            // 检查是否是更好的路径
            auto bestIt = gCost.find(neighborKey);
            bool isBetter = false;
            if (bestIt == gCost.end()) {
                isBetter = true;
            } else if (newWeightCost < bestIt->second) {
                isBetter = true;
            }

            if (isBetter) {
                gCost[neighborKey] = newWeightCost;
                cameFrom[neighborKey] = currentKey;
                Node neighborNode(neighbor, neighbor.distance(target), newWeightCost, currentKey);
                openSet.push(neighborNode);
            }
        }
    }

    // 没有找到路径
    return {};
}

// ==================== Fallback: 安全下一步 ====================

std::vector<Point> findSafeNextStep(
    const Point& startPos,
    const std::vector<std::vector<float>>& weights) {

    Point bestNext = startPos;
    float bestWeight = std::numeric_limits<float>::max();

    for (const auto& neighbor : getNeighbors(startPos)) {
        if (!isValid(neighbor)) continue;

        float weight = getCellWeight(weights, neighbor);
        // 跳过危险位置
        if (weight == WEIGHT_COLLISION) continue;

        // 评分：优先权重（越小越好）
        if (weight < bestWeight) {
            bestWeight = weight;
            bestNext = neighbor;
        }
    }

    if (bestNext != startPos) {
        return {bestNext};
    }
    return {};
}

// ==================== 获取下一步移动方向 ====================

std::string getNextMove(
    const Point& startPos,
    const std::vector<Point>& path) {

    if (path.size() < 2) {
        return "";
    }

    // 计算下一步的方向
    Point next = path[0];  // path[0] 是当前位置
    int dx = next.x - startPos.x;
    int dy = next.y - startPos.y;

    if (dx > 0) return "right";
    if (dx < 0) return "left";
    if (dy > 0) return "down";
    if (dy < 0) return "up";

    return "";
}
