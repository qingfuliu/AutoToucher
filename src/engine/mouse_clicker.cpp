//
// Created by lqf on 24-3-17.
//
#include "engine/mouse_clicker.h"

namespace clsn {

MouseClicker &MouseClicker::operator=(const clsn::MouseClicker::Json &json) {
  m_minimum_ = -1;
  m_maximum_ - -1;
  if (json.IsInteger()) {
    m_minimum_ = json;
    return *this;
  } else if (!json.IsScalar()) {
    throw clsn::EngineException(
        "To specify a random value for the MouseClicker object, "
        "parameters need to be provided in the following format: \"minimum-maximum\"");
    return *this;
  }
  do {
    Json::StringType str = json;
    Json::StringType::size_type pos = str.find("-");
    if (Json::StringType::npos == pos) {
      break;
    }
    try {
      size_t temp_pos = 0;
      m_minimum_ = std::stoi(str, &temp_pos, 10);
      if (temp_pos != pos) {
        m_minimum_ = -1;
        break;
      }
      m_maximum_ = std::stoi(str.substr(temp_pos + 1));
    } catch (...) {
      m_minimum_ = -1;
      m_maximum_ = -1;
    }
  } while (0);

  if (m_minimum_ == -1 || m_maximum_ == -1 || m_maximum_ <= m_minimum_) {
    throw clsn::EngineException(
        "To specify a random value for the MouseClicker object, "
        "parameters need to be provided in the following format: \"minimum-maximum\"");
  }
  return *this;
}

void MouseClicker::Click() const {}

void MouseClicker::DoubleClick() const {
  if (m_maximum_ == -1 && m_minimum_ == -1) {
    throw clsn::EngineException("Please initialize object MouseClicker before calling it");
  }
  if (m_maximum_ != -1) {
    return;
  }
}


}  // namespace clsn