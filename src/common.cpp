#include "common.hpp"

namespace RndNumGen {
    std::mt19937_64 rng(std::random_device{}());
};