#pragma once
#include <chrono>

// Give namespace so not to confuse with juce::Timer
namespace Utils {

template <class TimeT = std::chrono::milliseconds, class ClockT = std::chrono::steady_clock>
class Timer {
  using timep_t = typename ClockT::time_point;
  timep_t _start = ClockT::now(), _end = {};

 public:
  void tick() {
    _end = timep_t{};
    _start = ClockT::now();
  }

  void tock() { _end = ClockT::now(); }

  template <class TT = TimeT>
  TT duration() const {
    return std::chrono::duration_cast<TT>(_end - _start);
  }
};

}  // namespace Utils