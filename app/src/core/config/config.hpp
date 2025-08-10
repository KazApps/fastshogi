#pragma once

#include <memory>

#include <types/engine_config.hpp>
#include <types/tournament.hpp>

namespace fastshogi::config {

inline std::unique_ptr<Tournament> TournamentConfig;
inline std::unique_ptr<std::vector<EngineConfiguration>> EngineConfigs;

}  // namespace fastshogi::config
