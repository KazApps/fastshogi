#pragma once

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <engine/usi_engine.hpp>

namespace fastshogi::engine {

bool isValidInfoLine(const std::string &infoLine) {
    std::istringstream iss(infoLine);
    std::string token;

    if (!(iss >> token) || token != "info") {
        std::cerr << "\r\nInvalid info line format: " << infoLine << std::endl;
        return false;
    }

    while (iss >> token) {
        if (token == "time" || token == "nps" || token == "score") {
            if (!(iss >> token)) {
                std::cerr << "\r\nNo value after token: " << token << std::endl;
                return false;
            }

            if (token.find('.') != std::string::npos) {
                std::cerr << "\r\nTime/NPS/Score value is not an integer: " << token << std::endl;
                return false;
            }
        }
    }

    return true;
}

inline bool compliant(int argc, char const *argv[]) {
    int step = 0;

    EngineConfiguration config;
    config.cmd  = argv[2];
    config.args = argc > 3 ? std::string(argv[3]) : "";

    auto executeStep = [&step](const std::string &description, const std::function<bool()> &action) {
        step++;

        std::cout << "Step " << step << ": " << description << "..." << std::flush;

        if (!action()) {
            std::cerr << "\r\033[1;31m Failed\033[0m Step " << step << ": " << description << std::endl;
            return false;
        }

        std::cout << "\r" << "\033[1;32m Passed\033[0m Step " << step << ": " << description << std::endl;

        return true;
    };

    UsiEngine usi_engine(config, false);

    std::vector<std::pair<std::string, std::function<bool()>>> steps = {
        {"Start the engine", [&usi_engine] { return usi_engine.start(/*cpus*/ std::nullopt).has_value(); }},
        {"Check if engine is ready", [&usi_engine] { return usi_engine.isready() == process::Status::OK; }},
        {"Check id name", [&usi_engine] { return usi_engine.idName().has_value(); }},
        {"Check id author", [&usi_engine] { return usi_engine.idAuthor().has_value(); }},
        {"Send usinewgame", [&usi_engine] { return usi_engine.usinewgame(); }},
        {"Set position to startpos", [&usi_engine] { return usi_engine.writeEngine("position startpos"); }},
        {"Check if engine is ready after startpos",
         [&usi_engine] { return usi_engine.isready() == process::Status::OK; }},
        {"Set position to fen",
         [&usi_engine] {
             return usi_engine.writeEngine(
                 "position fen 3r2k1/p5n1/1pq1p2p/2p3p1/2P1P1n1/1P1P2pP/PN1Q2K1/5R2 w - - 0 27");
         }},

        {"Check if engine is ready after fen", [&usi_engine] { return usi_engine.isready() == process::Status::OK; }},
        {"Send go wtime 100", [&usi_engine] { return usi_engine.writeEngine("go wtime 100"); }},
        {"Read bestmove", [&usi_engine] { return usi_engine.readEngine("bestmove") == process::Status::OK; }},
        {"Check if engine prints an info line", [&usi_engine] { return !usi_engine.lastInfoLine().empty(); }},
        {"Verify info line format is valid", [&usi_engine] { return isValidInfoLine(usi_engine.lastInfoLine()); }},
        {"Verify info line contains score",
         [&usi_engine] { return str_utils::contains(usi_engine.lastInfoLine(), "score"); }},
        {"Set position to black to move",
         [&usi_engine] {
             return usi_engine.writeEngine("position fen rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
         }},
        {"Send go btime 100", [&usi_engine] { return usi_engine.writeEngine("go btime 100"); }},
        {"Read bestmove after go btime 100",
         [&usi_engine] { return usi_engine.readEngine("bestmove") == process::Status::OK; }},
        {"Check if engine prints an info line after go btime 100",
         [&usi_engine] { return !usi_engine.lastInfoLine().empty(); }},
        {"Verify info line format is valid after go btime 100",
         [&usi_engine] { return isValidInfoLine(usi_engine.lastInfoLine()); }},
        {"Check if engine prints an info line with the score after go btime 100",
         [&usi_engine] { return str_utils::contains(usi_engine.lastInfoLine(), "score"); }},
        {"Send go wtime 100 winc 100 btime 100 binc 100",
         [&usi_engine] { return usi_engine.writeEngine("go wtime 100 winc 100 btime 100 binc 100"); }},
        {"Read bestmove after go wtime 100 winc 100 btime 100 binc 100",
         [&usi_engine] { return usi_engine.readEngine("bestmove") == process::Status::OK; }},
        {"Check if engine prints an info line after go wtime 100 winc 100",
         [&usi_engine] { return !usi_engine.lastInfoLine().empty(); }},
        {"Verify info line format is valid after go wtime 100 winc 100",
         [&usi_engine] { return isValidInfoLine(usi_engine.lastInfoLine()); }},
        {"Check if engine prints an info line with the score after go wtime 100 winc 100",
         [&usi_engine] { return str_utils::contains(usi_engine.lastInfoLine(), "score"); }},
        {"Send go btime 100 binc 100 wtime 100 winc 100",
         [&usi_engine] { return usi_engine.writeEngine("go btime 100 binc 100 wtime 100 winc 100"); }},
        {"Read bestmove after go btime 100 binc 100 wtime 100 winc 100",
         [&usi_engine] { return usi_engine.readEngine("bestmove") == process::Status::OK; }},
        {"Check if engine prints an info line after go btime 100 binc 100",
         [&usi_engine] { return !usi_engine.lastInfoLine().empty(); }},
        {"Verify info line format is valid after go btime 100 binc 100",
         [&usi_engine] { return isValidInfoLine(usi_engine.lastInfoLine()); }},
        {"Check if engine prints an info line with the score after go btime 100 binc 100",
         [&usi_engine] { return str_utils::contains(usi_engine.lastInfoLine(), "score"); }},
        {"Check if engine prints an info line after go btime 100 binc 100",
         [&usi_engine] { return str_utils::contains(usi_engine.lastInfoLine(), "score"); }},
        // Simulate a game
        {"Send usinewgame", [&usi_engine] { return usi_engine.usinewgame(); }},
        {"Set position to startpos", [&usi_engine] { return usi_engine.writeEngine("position startpos"); }},
        {"Send go wtime 100", [&usi_engine] { return usi_engine.writeEngine("go wtime 100 btime 100"); }},
        {"Read bestmove after go wtime 100 btime 100",
         [&usi_engine] {
             return usi_engine.readEngine("bestmove") == process::Status::OK && usi_engine.bestmove() != std::nullopt;
         }},
        {"Verify info line format is valid after go wtime 100 btime 100",
         [&usi_engine] { return isValidInfoLine(usi_engine.lastInfoLine()); }},
        {"Set position to startpos moves e2e4 e7e5",
         [&usi_engine] { return usi_engine.writeEngine("position startpos moves e2e4 e7e5"); }},
        {"Send go wtime 100 btime 100", [&usi_engine] { return usi_engine.writeEngine("go wtime 100 btime 100"); }},
        {"Read bestmove after position startpos moves e2e4 e7e5",
         [&usi_engine] {
             return usi_engine.readEngine("bestmove") == process::Status::OK && usi_engine.bestmove() != std::nullopt;
         }},
        {"Verify info line format is valid after position startpos moves e2e4 e7e5",
         [&usi_engine] { return isValidInfoLine(usi_engine.lastInfoLine()); }}};

    for (const auto &[description, action] : steps) {
        if (!executeStep(description, action)) {
            return false;
        }
    }

    std::cout << "Engine passed all compliance checks." << std::endl;

    return true;
}

}  // namespace fastshogi::engine
