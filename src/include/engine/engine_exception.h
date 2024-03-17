//
// Created by lqf on 24-3-17.
//

#ifndef AUTOTOUCHER_ENGINE_EXCEPTION_H
#define AUTOTOUCHER_ENGINE_EXCEPTION_H
#include <stdexcept>
namespace clsn {
class EngineException : public std::logic_error {
 public:
  EngineException(const std::string &msg) noexcept : std::logic_error(msg) {}
};
}  // namespace clsn

#endif  // AUTOTOUCHER_ENGINE_EXCEPTION_H
