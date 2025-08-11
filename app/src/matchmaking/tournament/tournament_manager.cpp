#include <matchmaking/tournament/tournament_manager.hpp>

#include <cli/cli.hpp>
#include <cli/cli_args.hpp>
#include <core/config/config.hpp>
#include <core/globals/globals.hpp>
#include <core/logger/logger.hpp>
#include <core/rand.hpp>
#include <matchmaking/tournament/gauntlet/gauntlet.hpp>
#include <matchmaking/tournament/roundrobin/roundrobin.hpp>
#include <stdexcept>

namespace fastshogi {

TournamentManager::TournamentManager() {}

TournamentManager::~TournamentManager() {
    atomic::stop = true;
    LOG_TRACE("~TournamentManager()");
}

void TournamentManager::start(const cli::Args& args) {
    const auto options = cli::OptionsParser(args);

    config::TournamentConfig = std::make_unique<config::Tournament>(options.getTournamentConfig());

    config::EngineConfigs = std::make_unique<std::vector<EngineConfiguration>>(options.getEngineConfigs());

    Logger::setLevel(config::TournamentConfig->log.level);
    Logger::setCompress(config::TournamentConfig->log.compress);
    Logger::openFile(config::TournamentConfig->log.file);
    Logger::setEngineComs(config::TournamentConfig->log.engine_coms);

    LOG_INFO("{}", cli::OptionsParser::Version);

    random::seed(config::TournamentConfig->seed);

    LOG_TRACE("Creating tournament...");

    std::unique_ptr<BaseTournament> tournament;
    switch (config::TournamentConfig->type) {
        case TournamentType::ROUNDROBIN:
            tournament = std::make_unique<RoundRobin>(options.getResults());
            break;
        case TournamentType::GAUNTLET:
            tournament = std::make_unique<Gauntlet>(options.getResults());
            break;
        default:
            throw std::runtime_error("Unsupported tournament type: " +
                                     std::to_string(static_cast<int>(config::TournamentConfig->type)));
    }

    LOG_INFO("Starting tournament...");
    tournament->start();
}
}  // namespace fastshogi
