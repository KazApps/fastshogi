#include <core/config/config.hpp>
#include <engine/usi_engine.hpp>
#include <types/engine_config.hpp>

#include <chrono>
#include <string_view>
#include <thread>

#include <doctest/doctest.hpp>

using namespace fastshogi;

namespace {

#ifdef _WIN64
const std::string_view path = "./app/tests/mock/engine/dummy_engine.exe";
#else
const std::string_view path = "./app/tests/mock/engine/dummy_engine";
#endif

class MockUsiEngine : public engine::UsiEngine {
   public:
    explicit MockUsiEngine(const EngineConfiguration& config, bool realtime_logging)
        : engine::UsiEngine(config, realtime_logging) {}
};

}  // namespace

TEST_SUITE("Usi Engine Communication Tests") {
    TEST_CASE("Test engine::UsiEngine Args Simple") {
        EngineConfiguration config;
        config.cmd  = path;
        config.args = "arg1 arg2 arg3";

        engine::UsiEngine usi_engine = engine::UsiEngine(config, false);

        CHECK(usi_engine.start(/*cpus*/ std::nullopt));

        for (const auto& line : usi_engine.output()) {
            std::cout << line.line << std::endl;
        }

        CHECK(usi_engine.output().size() == 11);
        CHECK(usi_engine.output()[0].line == "argv[1]: arg1");
        CHECK(usi_engine.output()[1].line == "argv[2]: arg2");
        CHECK(usi_engine.output()[2].line == "argv[3]: arg3");

        CHECK(usi_engine.idName().has_value());
        CHECK(usi_engine.idName().value() == "dummy_engine");

        CHECK(usi_engine.idAuthor().has_value());
        CHECK(usi_engine.idAuthor().value() == "fastshogi");
    }

    TEST_CASE("Test engine::UsiEngine Args Complex") {
        EngineConfiguration config;
        config.cmd = path;
        config.args =
            "--backend=multiplexing "
            "--backend-opts=\"backend=cuda-fp16,(gpu=0),(gpu=1),(gpu=2),(gpu=3)\" "
            "--weights=lc0/BT4-1024x15x32h-swa-6147500.pb.gz --minibatch-size=132 "
            "--nncache=50000000 --threads=5";

        engine::UsiEngine usi_engine = engine::UsiEngine(config, false);

        CHECK(usi_engine.start(/*cpus*/ std::nullopt));

        for (const auto& line : usi_engine.output()) {
            std::cout << line.line << std::endl;
        }

        CHECK(usi_engine.output().size() == 14);
        CHECK(usi_engine.output()[0].line == "argv[1]: --backend=multiplexing");
        CHECK(usi_engine.output()[1].line ==
              "argv[2]: --backend-opts=backend=cuda-fp16,(gpu=0),(gpu=1),(gpu=2),(gpu=3)");
        CHECK(usi_engine.output()[2].line == "argv[3]: --weights=lc0/BT4-1024x15x32h-swa-6147500.pb.gz");
        CHECK(usi_engine.output()[3].line == "argv[4]: --minibatch-size=132");
        CHECK(usi_engine.output()[4].line == "argv[5]: --nncache=50000000");
        CHECK(usi_engine.output()[5].line == "argv[6]: --threads=5");
    }

    TEST_CASE("Testing the EngineProcess class") {
        EngineConfiguration config;
        config.cmd  = path;
        config.args = "arg1 arg2 arg3";

        engine::UsiEngine usi_engine = engine::UsiEngine(config, false);

        CHECK(usi_engine.start(/*cpus*/ std::nullopt));

        CHECK(usi_engine.output().size() == 11);
        CHECK(usi_engine.output()[0].line == "argv[1]: arg1");
        CHECK(usi_engine.output()[1].line == "argv[2]: arg2");
        CHECK(usi_engine.output()[2].line == "argv[3]: arg3");

        auto usiSuccess = usi_engine.usi();
        CHECK(usiSuccess);

        auto usi       = usi_engine.usiok();
        auto usiOutput = usi_engine.output();

        CHECK(usi);
        CHECK(usiOutput.size() == 8);
        CHECK(usiOutput[0].line == "id name dummy_engine");
        CHECK(usiOutput[1].line == "id author fastshogi");
        CHECK(usiOutput[2].line == "option name Threads type spin default 1 min 1 max 1024");
        CHECK(usiOutput[3].line == "option name Hash type spin default 1 min 1 max 500000");
        CHECK(usiOutput[4].line == "option name MultiPV type spin default 1 min 1 max 256");
        CHECK(usiOutput[5].line == "line0");
        CHECK(usiOutput[6].line == "line1");
        CHECK(usiOutput[7].line == "usiok");
        CHECK(usi_engine.isready() == engine::process::Status::OK);

        CHECK(usi_engine.writeEngine("sleep"));
        const auto res = usi_engine.readEngine("done", std::chrono::milliseconds(100));
        CHECK(res == engine::process::Status::TIMEOUT);

        CHECK(usi_engine.writeEngine("sleep"));
        const auto res2 = usi_engine.readEngine("done", std::chrono::milliseconds(5000));
        CHECK(res2 == engine::process::Status::OK);
        CHECK(usi_engine.output().size() == 1);
        CHECK(usi_engine.output()[0].line == "done");
    }

    TEST_CASE("Testing the EngineProcess class with lower level class functions") {
        EngineConfiguration config;
        config.cmd = path;

        engine::UsiEngine usi_engine = engine::UsiEngine(config, false);

        CHECK(usi_engine.start(/*cpus*/ std::nullopt));

        CHECK(usi_engine.writeEngine("usi"));
        const auto res = usi_engine.readEngine("usiok");

        CHECK(res == engine::process::Status::OK);
        CHECK(usi_engine.output().size() == 8);
        CHECK(usi_engine.output()[0].line == "id name dummy_engine");
        CHECK(usi_engine.output()[1].line == "id author fastshogi");
        CHECK(usi_engine.output()[2].line == "option name Threads type spin default 1 min 1 max 1024");
        CHECK(usi_engine.output()[3].line == "option name Hash type spin default 1 min 1 max 500000");
        CHECK(usi_engine.output()[4].line == "option name MultiPV type spin default 1 min 1 max 256");
        CHECK(usi_engine.output()[5].line == "line0");
        CHECK(usi_engine.output()[6].line == "line1");
        CHECK(usi_engine.output()[7].line == "usiok");

        CHECK(usi_engine.writeEngine("isready"));
        const auto res2 = usi_engine.readEngine("readyok");
        CHECK(res2 == engine::process::Status::OK);
        CHECK(usi_engine.output().size() == 1);
        CHECK(usi_engine.output()[0].line == "readyok");

        CHECK(usi_engine.writeEngine("sleep"));
        const auto res3 = usi_engine.readEngine("done", std::chrono::milliseconds(100));
        CHECK(res3 == engine::process::Status::TIMEOUT);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        CHECK(usi_engine.writeEngine("sleep"));
        const auto res4 = usi_engine.readEngine("done", std::chrono::milliseconds(5000));
        CHECK(res4 == engine::process::Status::OK);
        CHECK(usi_engine.output().size() == 1);
        CHECK(usi_engine.output()[0].line == "done");
    }

    TEST_CASE("Restarting the engine") {
        EngineConfiguration config;
        config.cmd = path;

        std::unique_ptr<engine::UsiEngine> usi_engine = std::make_unique<MockUsiEngine>(config, false);

        CHECK(usi_engine->start(/*cpus*/ std::nullopt));

        CHECK(usi_engine->writeEngine("usi"));
        const auto res = usi_engine->readEngine("usiok");

        CHECK(res == engine::process::Status::OK);
        CHECK(usi_engine->output().size() == 8);
        CHECK(usi_engine->output()[0].line == "id name dummy_engine");
        CHECK(usi_engine->output()[1].line == "id author fastshogi");
        CHECK(usi_engine->output()[2].line == "option name Threads type spin default 1 min 1 max 1024");
        CHECK(usi_engine->output()[3].line == "option name Hash type spin default 1 min 1 max 500000");
        CHECK(usi_engine->output()[4].line == "option name MultiPV type spin default 1 min 1 max 256");
        CHECK(usi_engine->output()[5].line == "line0");
        CHECK(usi_engine->output()[6].line == "line1");
        CHECK(usi_engine->output()[7].line == "usiok");

        usi_engine = std::make_unique<MockUsiEngine>(config, false);

        CHECK(usi_engine->start(/*cpus*/ std::nullopt));

        CHECK(usi_engine->writeEngine("usi"));
        const auto res2 = usi_engine->readEngine("usiok");

        CHECK(res2 == engine::process::Status::OK);
        CHECK(usi_engine->output().size() == 8);
        CHECK(usi_engine->output()[0].line == "id name dummy_engine");
        CHECK(usi_engine->output()[1].line == "id author fastshogi");
        CHECK(usi_engine->output()[2].line == "option name Threads type spin default 1 min 1 max 1024");
        CHECK(usi_engine->output()[3].line == "option name Hash type spin default 1 min 1 max 500000");
        CHECK(usi_engine->output()[4].line == "option name MultiPV type spin default 1 min 1 max 256");
        CHECK(usi_engine->output()[5].line == "line0");
        CHECK(usi_engine->output()[6].line == "line1");
        CHECK(usi_engine->output()[7].line == "usiok");
    }

    TEST_CASE("Restarting the engine") {
        EngineConfiguration config;
        config.cmd = path;

        std::unique_ptr<engine::UsiEngine> usi_engine = std::make_unique<MockUsiEngine>(config, false);

        CHECK(usi_engine->start(/*cpus*/ std::nullopt));

        CHECK(usi_engine->writeEngine("usi"));
        const auto res = usi_engine->readEngine("usiok");

        CHECK(res == engine::process::Status::OK);
        CHECK(usi_engine->output().size() == 8);
        CHECK(usi_engine->output()[0].line == "id name dummy_engine");
        CHECK(usi_engine->output()[1].line == "id author fastshogi");
        CHECK(usi_engine->output()[2].line == "option name Threads type spin default 1 min 1 max 1024");
        CHECK(usi_engine->output()[3].line == "option name Hash type spin default 1 min 1 max 500000");
        CHECK(usi_engine->output()[4].line == "option name MultiPV type spin default 1 min 1 max 256");
        CHECK(usi_engine->output()[5].line == "line0");
        CHECK(usi_engine->output()[6].line == "line1");
        CHECK(usi_engine->output()[7].line == "usiok");

        usi_engine = std::make_unique<MockUsiEngine>(config, false);

        CHECK(usi_engine->start(/*cpus*/ std::nullopt));

        CHECK(usi_engine->writeEngine("usi"));
        const auto res2 = usi_engine->readEngine("usiok");

        CHECK(res2 == engine::process::Status::OK);
        CHECK(usi_engine->output().size() == 8);
        CHECK(usi_engine->output()[0].line == "id name dummy_engine");
        CHECK(usi_engine->output()[1].line == "id author fastshogi");
        CHECK(usi_engine->output()[2].line == "option name Threads type spin default 1 min 1 max 1024");
        CHECK(usi_engine->output()[3].line == "option name Hash type spin default 1 min 1 max 500000");
        CHECK(usi_engine->output()[4].line == "option name MultiPV type spin default 1 min 1 max 256");
        CHECK(usi_engine->output()[5].line == "line0");
        CHECK(usi_engine->output()[6].line == "line1");
        CHECK(usi_engine->output()[7].line == "usiok");
    }

    TEST_CASE("Sending usi options and expect Threads to be first") {
        EngineConfiguration config;
        config.cmd     = path;
        config.options = {
            {"Hash", "1600"},
            {"MultiPV", "3"},
            {"Threads", "4"},
        };

        std::unique_ptr<engine::UsiEngine> usi_engine = std::make_unique<MockUsiEngine>(config, false);

        CHECK(usi_engine->start(/*cpus*/ std::nullopt));

        CHECK(usi_engine->refreshUsi());
        const auto res = usi_engine->readEngine("option set: setoption name MultiPV value 3");

        CHECK(res == engine::process::Status::OK);
        CHECK(usi_engine->output().size() == 3);
        CHECK(usi_engine->output()[0].line == "option set: setoption name Threads value 4");
        CHECK(usi_engine->output()[1].line == "option set: setoption name Hash value 1600");
        CHECK(usi_engine->output()[2].line == "option set: setoption name MultiPV value 3");
    }
}
