#include <engine/usi_engine.hpp>

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

#include <core/config/config.hpp>
#include <core/helper.hpp>
#include <core/logger/logger.hpp>
#include <engine/option/option_factory.hpp>
#include <shogi/shogi.hpp>

namespace fastshogi::engine {

// A counting semaphore to limit the number of threads
class CountingSemaphore {
   public:
    CountingSemaphore(int max_) : max__(max_), current_(0) {}

    void acquire() {
        std::unique_lock<std::mutex> lock(mtx_);
        cond_.wait(lock, [this] { return current_ < max__; });
        ++current_;
    }

    void release() {
        std::lock_guard<std::mutex> lock(mtx_);
        --current_;
        cond_.notify_one();
    }

   private:
    std::mutex mtx_;
    std::condition_variable cond_;
    int max__;
    int current_;
};

// RAII class to acquire and release the semaphore
class AcquireSemaphore {
   public:
    explicit AcquireSemaphore(CountingSemaphore &semaphore) : semaphore_(semaphore) { semaphore_.acquire(); }

    ~AcquireSemaphore() { semaphore_.release(); }

   private:
    CountingSemaphore &semaphore_;
};

namespace {
CountingSemaphore semaphore(16);

}  // namespace

UsiEngine::UsiEngine(const EngineConfiguration &config, bool realtime_logging) : realtime_logging_(realtime_logging) {
    loadConfig(config);
    output_.reserve(100);

    process_.setRealtimeLogging(realtime_logging_);
}

process::Status UsiEngine::isready(std::chrono::milliseconds threshold) {
    const auto is_alive = process_.alive();
    if (is_alive != process::Status::OK) return is_alive;

    LOG_TRACE_THREAD("Pinging engine {}", config_.name);

    writeEngine("isready");

    setupReadEngine();

    std::vector<process::Line> output;
    const auto res = process_.readOutput(output, "readyok", threshold);

    // print output in case we are using delayed logging
    if (!realtime_logging_) {
        for (const auto &line : output) {
            Logger::readFromEngine(line.line, line.time, config_.name, line.std == process::Standard::ERR);
        }
    }

    if (res != process::Status::OK) {
        if (!atomic::stop) {
            LOG_TRACE_THREAD("Engine {} didn't respond to isready", config_.name);
            Logger::print<Logger::Level::WARN>("Warning; Engine {} is not responsive", config_.name);
        }

        return res;
    }

    LOG_TRACE_THREAD("Engine {} is {}", config_.name, "responsive");

    return res;
}

bool UsiEngine::position(const std::vector<std::string> &moves, const std::string &fen) {
    auto position = fmt::format("position {}", fen == "startpos" ? "startpos" : ("fen " + fen));

    if (!moves.empty()) {
        position += " moves";
        for (const auto &move : moves) {
            position += " " + move;
        }
    }

    return writeEngine(position);
}

bool UsiEngine::go(const TimeControl &our_tc, const TimeControl &enemy_tc, shogi::Color stm) {
    std::stringstream input;
    input << "go";

    if (config_.limit.nodes) {
        input << " nodes " << config_.limit.nodes;
    }

    if (config_.limit.plies) {
        input << " depth " << config_.limit.plies;
    }

    // We cannot use st and tc together
    if (our_tc.isFixedTime()) {
        input << " movetime " << our_tc.getFixedTime();
        return writeEngine(input.str());
    }

    const auto &white = stm == shogi::Color::WHITE ? our_tc : enemy_tc;
    const auto &black = stm == shogi::Color::WHITE ? enemy_tc : our_tc;

    if (our_tc.isTimed() || our_tc.isIncrement()) {
        if (white.isTimed() || white.isIncrement()) {
            input << " wtime " << white.getTimeLeft();
        }

        if (black.isTimed() || black.isIncrement()) {
            input << " btime " << black.getTimeLeft();
        }
    }

    if (our_tc.isIncrement()) {
        if (white.isIncrement()) {
            input << " winc " << white.getIncrement();
        }

        if (black.isIncrement()) {
            input << " binc " << black.getIncrement();
        }
    }

    return writeEngine(input.str());
}

bool UsiEngine::usinewgame() {
    LOG_TRACE_THREAD("Sending usinewgame to engine {}", config_.name);
    auto res = writeEngine("usinewgame");

    if (!res) {
        LOG_WARN_THREAD("Failed to send usinewgame to engine {}", config_.name);
        return false;
    }

    return isready(usinewgame_time_) == process::Status::OK;
}

std::optional<std::string> UsiEngine::idName() {
    if (!usi()) {
        Logger::print<Logger::Level::WARN>("Warning; Engine {} didn't respond to usi.", config_.name);

        return std::nullopt;
    }

    if (!usiok()) {
        Logger::print<Logger::Level::WARN>("Warning; Engine {} didn't respond to usi.", config_.name);
        return std::nullopt;
    }

    // get id name
    for (const auto &line : output_) {
        if (line.line.find("id name") != std::string::npos) {
            // everything after id name
            return line.line.substr(line.line.find("id name") + 8);
        }
    }

    return std::nullopt;
}

std::optional<std::string> UsiEngine::idAuthor() {
    if (!usi()) {
        Logger::print<Logger::Level::WARN>("Warning; Engine {} didn't respond to usi.", config_.name);

        return std::nullopt;
    }

    if (!usiok()) {
        Logger::print<Logger::Level::WARN>("Warning; Engine {} didn't respond to usi.", config_.name);

        return std::nullopt;
    }

    // get id author
    for (const auto &line : output_) {
        if (line.line.find("id author") != std::string::npos) {
            // everything after id author
            return line.line.substr(line.line.find("id author") + 10);
        }
    }

    return std::nullopt;
}

bool UsiEngine::usi() {
    LOG_TRACE_THREAD("Sending usi to engine {}", config_.name);
    const auto res = writeEngine("usi");

    if (!res) {
        LOG_WARN_THREAD("Failed to send usi to engine {}", config_.name);
        return false;
    }

    return res;
}

bool UsiEngine::usiok(std::chrono::milliseconds threshold) {
    LOG_TRACE_THREAD("Waiting for usiok from engine {}", config_.name);

    const auto res = readEngine("usiok", threshold) == process::Status::OK;

    for (const auto &line : output_) {
        if (!realtime_logging_) {
            Logger::readFromEngine(line.line, line.time, config_.name, line.std == process::Standard::ERR);
        }

        auto option = USIOptionFactory::parseUSIOptionLine(line.line);

        if (option != nullptr) {
            usi_options_.addOption(std::move(option));
        }
    }

    if (!res) LOG_WARN_THREAD("Engine {} did not respond to usiok in time.", config_.name);

    return res;
}

void UsiEngine::loadConfig(const EngineConfiguration &config) { config_ = config; }

void UsiEngine::quit() {
    if (!initialized_) return;
    LOG_TRACE_THREAD("Sending quit to engine {}", config_.name);
    writeEngine("quit");
}

void UsiEngine::sendSetoption(const std::string &name, const std::string &value) {
    auto option = usi_options_.getOption(name);

    if (!option.has_value()) {
        Logger::print<Logger::Level::WARN>("Warning; {} doesn't have option {}", config_.name, name);

        return;
    }

    if (!option.value()->isValid(value)) {
        Logger::print<Logger::Level::WARN>("Warning; Invalid value for option {}; {}", name, value);

        return;
    }

    LOG_TRACE_THREAD("Sending setoption to engine {} {} {}", config_.name, name, value);

    if (option.value()->getType() == USIOption::Type::Button) {
        if (value != "true") {
            return;
        }

        if (!writeEngine(fmt::format("setoption name {}", name))) {
            LOG_WARN_THREAD("Failed to send setoption to engine {} {}", config_.name, name);
            return;
        }

        option.value()->setValue(value);
    }

    if (!writeEngine(fmt::format("setoption name {} value {}", name, value))) {
        LOG_WARN_THREAD("Failed to send setoption to engine {} {} {}", config_.name, name, value);
        return;
    }

    option.value()->setValue(value);
}

tl::expected<bool, std::string> UsiEngine::start(const std::optional<std::vector<int>> &cpus) {
    if (initialized_) return true;

    AcquireSemaphore semaphore_acquire(semaphore);

    const auto path = config_.cmd;
    LOG_TRACE_THREAD("Starting engine {} at {}", config_.name, path);

    // Creates the engine process and sets the pipes
    if (process_.init(config_.dir, path, config_.args, config_.name) != process::Status::OK) {
        return tl::make_unexpected("Couldn't start engine process");
    }

    if (cpus) {
        setCpus(*cpus);
    }

    initialized_ = true;

    // Wait for the engine to start
    if (!usi()) {
        return tl::make_unexpected("Couldn't write usi to engine");
    }

    if (!usiok(startup_time_)) {
        return tl::make_unexpected("Engine didn't respond to usiok after startup");
    }

    return true;
}

bool UsiEngine::refreshUsi() {
    LOG_TRACE_THREAD("Refreshing engine {}", config_.name);

    if (!usinewgame()) {
        LOG_WARN_THREAD("Engine {} failed to start/refresh (usinewgame).", config_.name);
        return false;
    }

    // Reorder to send Threads option first, to help multi-threaded configurations

    auto options = config_.options;
    std::stable_partition(options.begin(), options.end(), [](const auto &p) { return p.first == "Threads"; });

    for (const auto &option : options) {
        try {
            sendSetoption(option.first, option.second);

        } catch (const std::exception &e) {
            Logger::print<Logger::Level::WARN>("Warning; Failed to set option {} with value {} for engine {}: {}",
                                               option.first, option.second, config_.name, e.what());
        }
    }

    LOG_TRACE_THREAD("Engine {} refreshed.", config_.name);

    return true;
}

process::Status UsiEngine::readEngine(std::string_view last_word, std::chrono::milliseconds threshold) {
    setupReadEngine();
    return process_.readOutput(output_, last_word, threshold);
}

void UsiEngine::writeLog() const {
    for (const auto &line : output_) {
        Logger::readFromEngine(line.line, line.time, config_.name, line.std == process::Standard::ERR);
    }
}

std::string UsiEngine::lastInfoLine() const {
    std::string fallback;

    // iterate backwards over the output and save the info line
    for (auto it = output_.rbegin(); it != output_.rend(); ++it) {
        const auto &line = it->line;

        // only consider info lines with score and (multipv 1 if present)
        if (line.find("info") == std::string::npos || line.find(" score ") == std::string::npos ||
            (line.find(" multipv ") != std::string::npos && line.find(" multipv 1") == std::string::npos)) {
            continue;
        }

        bool isBound = (line.find("lowerbound") != std::string::npos || line.find("upperbound") != std::string::npos);

        // prefer exact scores
        if (!isBound) return line;

        // save as fallback if we don’t find an exact one
        if (fallback.empty()) fallback = line;
    }

    return fallback;
}

bool UsiEngine::writeEngine(const std::string &input) {
    Logger::writeToEngine(input, "", config_.name);
    return process_.writeInput(input + "\n") == process::Status::OK;
}

std::optional<std::string> UsiEngine::bestmove() const {
    if (output_.empty()) {
        Logger::print<Logger::Level::WARN>("Warning; No output from {}", config_.name);

        return std::nullopt;
    }

    const auto bm = str_utils::findElement<std::string>(str_utils::splitString(output_.back().line, ' '), "bestmove");

    if (!bm.has_value()) {
        Logger::print<Logger::Level::WARN>("Warning; No bestmove found in the last line from {}", config_.name);

        return std::nullopt;
    }

    return bm.value();
}

std::vector<std::string> UsiEngine::lastInfo() const {
    const auto last_info = lastInfoLine();

    if (last_info.empty()) {
        Logger::print<Logger::Level::WARN>("Warning; Last info string with score not found from {}", config_.name);
        return {};
    }

    return str_utils::splitString(last_info, ' ');
}

ScoreType UsiEngine::lastScoreType() const {
    auto score = str_utils::findElement<std::string>(lastInfo(), "score").value_or("ERR");

    if (score == "ERR") return ScoreType::ERR;
    if (score == "cp") return ScoreType::CP;
    if (score == "mate") return ScoreType::MATE;

    return ScoreType::ERR;
}

std::chrono::milliseconds UsiEngine::lastTime() const {
    std::vector<std::string> last_reported_time_info;

    for (auto it = output_.rbegin(); it != output_.rend(); ++it) {
        const auto &line = it->line;

        if (line.find("info") == std::string::npos || line.find("time") == std::string::npos) {
            continue;
        }

        last_reported_time_info = str_utils::splitString(line, ' ');
        break;
    }

    const auto time = str_utils::findElement<int>(last_reported_time_info, "time").value_or(0);

    return std::chrono::milliseconds(time);
}

int UsiEngine::lastScore() const {
    const auto score = lastScoreType();

    if (score == ScoreType::ERR) {
        return 0;
    }

    return str_utils::findElement<int>(lastInfo(), lastScoreType() == ScoreType::CP ? "cp" : "mate").value_or(0);
}

bool UsiEngine::outputIncludesBestmove() const {
    for (const auto &line : output_) {
        if (line.line.find("bestmove") != std::string::npos) return true;
    }

    return false;
}

}  // namespace fastshogi::engine
