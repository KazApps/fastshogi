#pragma once

#include <core/helper.hpp>

namespace fastshogi::config {

struct MaxMovesAdjudication {
    int move_count = 1;

    bool enabled = false;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MaxMovesAdjudication, move_count, enabled)

}  // namespace fastshogi::config