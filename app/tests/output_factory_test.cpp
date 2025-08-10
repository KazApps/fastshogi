#include <matchmaking/output/output_factory.hpp>

#include <doctest/doctest.hpp>

namespace fastshogi {

TEST_SUITE("Output Factory") {
    TEST_CASE("Return Cutechess Output") {
        auto output = OutputFactory::create(OutputType::CUTECHESS, false);

        CHECK(output != nullptr);
        CHECK(output->getType() == OutputType::CUTECHESS);
    }

    TEST_CASE("Return Fastshogi Output") {
        auto output = OutputFactory::create(OutputType::FASTSHOGI, false);

        CHECK(output != nullptr);
        CHECK(output->getType() == OutputType::FASTSHOGI);
    }

    TEST_CASE("Return Fastshogi Output by default") {
        auto output = OutputFactory::create(OutputType::NONE, false);

        CHECK(output != nullptr);
        CHECK(output->getType() == OutputType::FASTSHOGI);
    }

    TEST_CASE("Get Cutechess Output Type") {
        auto type = OutputFactory::getType("cutechess");

        CHECK(type == OutputType::CUTECHESS);
    }

    TEST_CASE("Get Fastshogi Output Type") {
        auto type = OutputFactory::getType("fastshogi");

        CHECK(type == OutputType::FASTSHOGI);
    }

    TEST_CASE("Get Fastshogi Output Type by default") {
        auto type = OutputFactory::getType("none");

        CHECK(type == OutputType::FASTSHOGI);
    }
}

}  // namespace fastshogi