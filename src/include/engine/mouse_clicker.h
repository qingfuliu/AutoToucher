//
// Created by lqf on 24-3-17.
//

#ifndef AUTOTOUCHER_MOUSE_CLICKER_H
#define AUTOTOUCHER_MOUSE_CLICKER_H
#include <string>
#include "common.h"
#include "configurator/auto_config.h"
#include "engine_exception.h"

namespace clsn {

class MouseClicker {
  using Json = json::Json::value;

 public:
  MouseClicker() noexcept = default;

  MouseClicker(const Json &json) { this->operator=(json); }

  ~MouseClicker() = default;

  MouseClicker &operator=(const Json &json);

  void Click() const;

  void DoubleClick() const;

 private:


 private:
  int32_t m_minimum_{-1};
  int32_t m_maximum_{-1};
};
}  // namespace clsn

#endif  // AUTOTOUCHER_MOUSE_CLICKER_H
