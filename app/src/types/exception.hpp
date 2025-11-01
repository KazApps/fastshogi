#pragma once

#include <exception>
#include <string>

namespace fastshogi {

class fastshogi_exception : public std::exception {
   public:
    explicit fastshogi_exception(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }

   private:
    std::string message_;
};

}  // namespace fastshogi
