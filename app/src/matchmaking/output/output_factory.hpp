#pragma once

#include <matchmaking/output/output_cutechess.hpp>
#include <matchmaking/output/output_fastshogi.hpp>
#include <types/tournament.hpp>

namespace fastshogi {

class OutputFactory {
   public:
    [[nodiscard]] static std::unique_ptr<IOutput> create(OutputType type, bool report_penta) {
        switch (type) {
            case OutputType::FASTSHOGI:
                return std::make_unique<Fastshogi>(report_penta);
            case OutputType::CUTECHESS:
                return std::make_unique<Cutechess>();
            default:
                return std::make_unique<Fastshogi>(report_penta);
        }
    }

    [[nodiscard]] static OutputType getType(const std::string& type) {
        if (type == "fastshogi") {
            return OutputType::FASTSHOGI;
        }

        if (type == "cutechess") {
            return OutputType::CUTECHESS;
        }

        return OutputType::FASTSHOGI;
    }
};

}  // namespace fastshogi
