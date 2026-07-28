#ifndef ACTIVESTATUS_H
#define ACTIVESTATUS_H
#include <string>
#include <optional>

struct ActiveStatus {
  std::string userName;
  bool active = false;
  std::optional<std::string> lastActiveAt;
};

#endif