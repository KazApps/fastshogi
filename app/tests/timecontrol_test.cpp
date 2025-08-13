#include <game/timecontrol/timecontrol.hpp>

#include <doctest/doctest.hpp>

using namespace fastshogi;

TEST_SUITE("TimeControl") {
    TEST_CASE("time+increment") {
        TimeControl::Limits limits;
        limits.time       = 10000;
        limits.increment  = 100;
        limits.timemargin = 100;

        TimeControl tc(limits);

        CHECK(tc.updateTime(5555) == true);
        CHECK(tc.getTimeLeft() == limits.time + limits.increment - 5555 + limits.increment);

        CHECK(tc.updateTime(4745) == true);
        CHECK(tc.getTimeLeft() == limits.increment);

        CHECK(tc.updateTime(10251) == false);
        CHECK(tc.getTimeLeft() == limits.increment - 10251);
    }

    TEST_CASE("Fixed time") {
        TimeControl::Limits limits;
        limits.fixed_time = 5000;
        limits.timemargin = 200;

        TimeControl tc(limits);

        CHECK(tc.updateTime(limits.fixed_time + limits.timemargin - 1) == true);
        CHECK(tc.getTimeLeft() == limits.fixed_time);

        CHECK(tc.updateTime(limits.fixed_time + limits.timemargin) == true);
        CHECK(tc.getTimeLeft() == limits.fixed_time);

        CHECK(tc.updateTime(limits.fixed_time + limits.timemargin + 1) == false);
        CHECK(tc.getTimeLeft() == limits.fixed_time - (limits.fixed_time + limits.timemargin + 1));
    }

    TEST_CASE("Fixed depth/nodes") {
        TimeControl::Limits limits;

        TimeControl tc(limits);

        CHECK(tc.updateTime(523199) == true);
        CHECK(tc.getTimeLeft() == 0);
    }
}
