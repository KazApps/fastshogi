#pragma once

#include <core/helper.hpp>

namespace fastshogi::config {

struct ResignAdjudication {
    int move_count = 1;
    int score      = 0;
    bool twosided  = false;

    bool enabled = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ResignAdjudication, move_count, score, twosided, enabled)

}  // namespace fastshogi::config
