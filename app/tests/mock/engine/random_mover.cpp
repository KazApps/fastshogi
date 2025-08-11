#include <random>

#include "../../../third_party/shogi.hpp"

using namespace shogi;

std::mt19937 gen;

void initialize_rng(int argc, char *argv[]) {
    if (argc > 1) {
        std::cout << "Using provided seed: " << argv[1] << std::endl;
        unsigned int seed = std::stoi(argv[1]);
        gen.seed(seed);
    } else {
        std::random_device rd;
        auto seed = rd();
        gen.seed(seed);
        std::cout << "Using random seed: " << seed << std::endl;
    }
}

int random_number(int min, int max) {
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

Move random_move(const Board &board) {
    auto moves = Movelist();
    movegen::legalmoves(moves, board);

    return moves[random_number(0, moves.size() - 1)];
}

void usi_line(Board &board, const std::string &line) {
    auto tokens = utils::splitString(line, ' ');

    if (tokens.empty())
        return;
    else if (tokens[0] == "position") {
        // parse position
        if (tokens[1] == "startpos") {
            board = Board();
            if (tokens.size() > 2 && tokens[2] == "moves") {
                for (int i = 3; i < tokens.size(); i++) {
                    board.makeMove(usi::usiToMove(board, std::string(tokens[i])));
                }
            }
        } else if (tokens[1] == "fen") {
            std::string fen = "";
            // parse until "moves"
            int i = 2;
            for (; i < tokens.size(); i++) {
                if (tokens[i] == "moves") {
                    i++;
                    break;
                }
                fen += std::string(tokens[i]) + " ";
            }

            board.setFen(fen);

            for (; i < tokens.size(); i++) {
                board.makeMove(usi::usiToMove(board, std::string(tokens[i])));
            }
        }
    } else if (tokens[0] == "usi") {
        std::cout << "id name random_move" << std::endl;
        std::cout << "id author fastshogi" << std::endl;
        std::cout << "option name Hash type spin default 16 min 1 max 33554432" << std::endl;
        std::cout << "usiok" << std::endl;
    } else if (tokens[0] == "isready") {
        std::cout << "readyok" << std::endl;
    } else if (tokens[0] == "usinewgame") {
        board = Board();
    } else if (tokens[0] == "go") {
        const auto move = random_move(board);
        std::cout << "info depth 1 pv " << usi::moveToUsi(move) << " score cp 0 " << std::endl;
        std::cout << "bestmove " << usi::moveToUsi(move) << std::endl;
    }
}

void usi_loop() {
    std::string input;
    std::cin >> std::ws;

    Board board = Board();

    while (true) {
        if (!std::getline(std::cin, input) && std::cin.eof()) {
            input = "quit";
        }

        if (input == "quit") {
            return;
        } else {
            usi_line(board, input);
        }
    }
}

int main(int argc, char *argv[]) {
    initialize_rng(argc, argv);
    usi_loop();
}
