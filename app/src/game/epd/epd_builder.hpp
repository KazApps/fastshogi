#pragma once

#include <sstream>
#include <string>

#include <shogi.hpp>

#include <core/config/config.hpp>
#include <matchmaking/match/match.hpp>
#include <types/tournament.hpp>

namespace fastshogi::epd {

class EpdBuilder {
   public:
    EpdBuilder(const VariantType &variant, const MatchData &match) {
        shogi::Board board = shogi::Board();
        board.set960(variant == VariantType::FRC);
        board.setFen(match.fen);

        for (const auto &move : match.moves) {
            const auto illegal = !move.legal;

            if (illegal) break;

            board.makeMove<true>(shogi::uci::uciToMove(board, move.move));
        }

        epd << board.getEpd() << "\n";
    }

    // Get the newly created epd
    [[nodiscard]] std::string get() const noexcept { return epd.str(); }

   private:
    std::stringstream epd;
};

}  // namespace fastshogi::epd
