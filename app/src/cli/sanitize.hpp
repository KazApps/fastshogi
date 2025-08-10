#pragma once

#include <types/engine_config.hpp>
#include <types/tournament.hpp>

namespace fastshogi::cli {

void sanitize(config::Tournament&);

void sanitize(std::vector<EngineConfiguration>&);

}  // namespace fastshogi::cli
