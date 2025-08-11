#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "usioption.hpp"

namespace fastshogi {

class USIOptions {
   public:
    void addOption(std::unique_ptr<USIOption> option) { options.push_back(std::move(option)); }

    std::optional<USIOption*> getOption(const std::string& name) {
        for (auto& option : options) {
            if (option->getName() == name) {
                return option.get();
            }
        }

        return std::nullopt;
    }

    std::optional<USIOption*> getOption(const std::string& name) const {
        for (const auto& option : options) {
            if (option->getName() == name) {
                return option.get();
            }
        }

        return std::nullopt;
    }

    const std::vector<std::unique_ptr<USIOption>>& getOptions() { return options; }

   private:
    std::vector<std::unique_ptr<USIOption>> options;
};

}  // namespace fastshogi
