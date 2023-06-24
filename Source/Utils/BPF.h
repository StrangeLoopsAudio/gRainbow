#pragma once

// Ripped from essentia utils
class BPF {
 protected:
  std::vector<float> _xPoints;
  std::vector<float> _yPoints;
  std::vector<float> _slopes;

 public:
  BPF() {}
  BPF(std::vector<float> xPoints, std::vector<float> yPoints) { init(xPoints, yPoints); }
  void init(std::vector<float> xPoints, std::vector<float> yPoints) {
    _xPoints = xPoints;
    _yPoints = yPoints;

    jassert(_xPoints.size() == _yPoints.size());
    jassert(_xPoints.size() >= 2);
    for (size_t i = 1; i < _xPoints.size(); ++i) {
      jassert(_xPoints[i - 1] < _xPoints[i]);
    }

    _slopes.resize(_xPoints.size() - 1);

    for (size_t j = 1; j < _xPoints.size(); ++j) {
      // this never gives a division by zero as we checked just before that
      // x[i-1] < x[i]
      _slopes[j - 1] = (_yPoints[j] - _yPoints[j - 1]) / (_xPoints[j] - _xPoints[j - 1]);
    }
  }

  inline float operator()(float x) {
    jassert(x >= _xPoints[0]);
    jassert(x <= _xPoints.back());

    std::vector<float>::size_type j = 0;
    while (x > _xPoints[j + 1]) {
      j += 1;
    }

    return (x - _xPoints[j]) * _slopes[j] + _yPoints[j];
  }
};
