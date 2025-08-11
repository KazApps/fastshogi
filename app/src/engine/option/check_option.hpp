#pragma once

#include "usioption.hpp"

namespace fastshogi {

class CheckOption : public USIOption {
   public:
    CheckOption(const std::string& name) : name(name) {}

    std::string getName() const override { return name; }

    void setValue(const std::string& value) override {
        if (isValid(value)) {
            this->value = (value == "true");
        }
    }

    std::string getValue() const override { return value ? "true" : "false"; }

    bool isValid(const std::string& value) const override { return value == "true" || value == "false"; }

    Type getType() const override { return Type::Check; }

   private:
    std::string name;
    bool value;
};

}  // namespace fastshogi
