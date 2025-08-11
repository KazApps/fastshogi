#pragma once

#include <string>
#include <vector>

#include <shogi/shogi.hpp>

namespace fastshogi::book {
struct Opening {
    Opening() = default;
    Opening(const std::string& fen_epd, const std::vector<shogi::Move>& moves) : fen_epd(fen_epd), moves(moves) {}

    std::string fen_epd            = shogi::constants::STARTPOS;
    std::vector<shogi::Move> moves = {};
};
}  // namespace fastshogi::book
