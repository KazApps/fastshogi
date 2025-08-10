#include <matchmaking/syzygy.hpp>

#include <core/filesystem/file_system.hpp>

#include <pyrrhic/tbprobe.h>

#include <cassert>
#include <stdexcept>

namespace fastshogi {

int initSyzygy(const std::string_view syzygyDirs) {
    const bool success = tb_init(syzygyDirs.data());
    if (!success) {
        return 0;
    }

    return TB_LARGEST;
}

void tearDownSyzygy() { tb_free(); }

bool canProbeSyzgyWdl(const shogi::Board& board) {
    if (board.halfMoveClock() != 0) {
        return false;
    }
    if (!board.castlingRights().isEmpty()) {
        return false;
    }
    if (board.occ().count() > TB_LARGEST) {
        return false;
    }

    return true;
}

shogi::GameResult probeSyzygyWdl(const shogi::Board& board, const bool ignore50MoveRule) {
    assert(canProbeSyzgyWdl(board));
    // We now assume that the half move clock is 0 and the castling rights are empty.
    // If these conditions are not fulfilled, the probe result may be incorrect.

    const unsigned probeResult =
        tb_probe_wdl(board.us(shogi::Color::WHITE).getBits(), board.us(shogi::Color::BLACK).getBits(),
                     board.pieces(shogi::PieceType::KING).getBits(), board.pieces(shogi::PieceType::QUEEN).getBits(),
                     board.pieces(shogi::PieceType::ROOK).getBits(), board.pieces(shogi::PieceType::BISHOP).getBits(),
                     board.pieces(shogi::PieceType::KNIGHT).getBits(), board.pieces(shogi::PieceType::PAWN).getBits(),
                     board.enpassantSq().index(), board.sideToMove() == shogi::Color::WHITE);

    if (probeResult == TB_RESULT_FAILED) {
        return shogi::GameResult::NONE;
    }

    switch (probeResult) {
        case TB_WIN:
            return shogi::GameResult::WIN;

        case TB_CURSED_WIN:
            return ignore50MoveRule ? shogi::GameResult::WIN : shogi::GameResult::DRAW;

        case TB_DRAW:
            return shogi::GameResult::DRAW;

        case TB_BLESSED_LOSS:
            return ignore50MoveRule ? shogi::GameResult::LOSE : shogi::GameResult::DRAW;

        case TB_LOSS:
            return shogi::GameResult::LOSE;
    }

    // Should be unreachable.
    assert(false);

    return shogi::GameResult::NONE;
}

}  // namespace fastshogi
