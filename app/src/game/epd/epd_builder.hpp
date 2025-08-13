#pragma once

#include <sstream>
#include <string>

#include <matchmaking/match/match.hpp>
#include <shogi/shogi.hpp>

namespace fastshogi::epd {

class EpdBuilder {
   public:
    EpdBuilder(const MatchData &match) {
        shogi::Board board = shogi::Board();
        board.setFen(match.fen);

        for (const auto &move : match.moves) {
            const auto illegal = !move.legal;

            if (illegal) break;

            board.makeMove<true>(shogi::usi::usiToMove(board, move.move));
        }

        epd << board.getEpd() << "\n";
    }

    // Get the newly created epd
    [[nodiscard]] std::string get() const noexcept { return epd.str(); }

   private:
    std::stringstream epd;
};

}  // namespace fastshogi::epd
