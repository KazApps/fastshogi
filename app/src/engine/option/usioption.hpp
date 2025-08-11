#pragma once

#include <string>

namespace fastshogi {

class USIOption {
   public:
    enum class Type { Button, Check, Combo, Spin, String };

    virtual ~USIOption()                                 = default;
    virtual std::string getName() const                  = 0;
    virtual void setValue(const std::string& value)      = 0;
    virtual std::string getValue() const                 = 0;
    virtual bool isValid(const std::string& value) const = 0;
    virtual Type getType() const                         = 0;
};

}  // namespace fastshogi
