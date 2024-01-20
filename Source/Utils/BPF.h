#pragma once
#include <vector>
#include <juce_core/system/juce_PlatformDefs.h>
#include <stddef.h>

// Ripped from essentia utils
class BPF {
 protected:
  std::vector<float> mXPoints;
  std::vector<float> mYPoints;
  std::vector<float> mSlopes;

 public:
  BPF() {}
  BPF(std::vector<float> xPoints, std::vector<float> yPoints) { init(xPoints, yPoints); }
  void init(std::vector<float> xPoints, std::vector<float> yPoints) {
    mXPoints = xPoints;
    mYPoints = yPoints;

    jassert(mXPoints.size() == mYPoints.size());
    jassert(mXPoints.size() >= 2);
    for (size_t i = 1; i < mXPoints.size(); ++i) {
      jassert(mXPoints[i - 1] < mXPoints[i]);
    }

    mSlopes.resize(mXPoints.size() - 1);

    for (size_t j = 1; j < mXPoints.size(); ++j) {
      // this never gives a division by zero as we checked just before that
      // x[i-1] < x[i]
      mSlopes[j - 1] = (mYPoints[j] - mYPoints[j - 1]) / (mXPoints[j] - mXPoints[j - 1]);
    }
  }

  inline float operator()(float x) {
    jassert(x >= mXPoints[0]);
    jassert(x <= mXPoints.back());

    std::vector<float>::size_type j = 0;
    while (x > mXPoints[j + 1]) {
      j += 1;
    }

    return (x - mXPoints[j]) * mSlopes[j] + mYPoints[j];
  }
};
