#pragma once

#include <cli/cli_args.hpp>

namespace fastshogi {

// Manages the tournament, currently wraps round robin but can be extended to support
// different tournament types
class TournamentManager {
   public:
    TournamentManager();
    ~TournamentManager();

    void start(const cli::Args& args);
};

}  // namespace fastshogi
