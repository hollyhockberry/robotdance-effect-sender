// Copyright (c) 2022 Inaba (@hollyhockberry)
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#ifndef AVERAGE_H_
#define AVERAGE_H_

// 移動平均
class Average {
  const int _points;
  float* _data;
  float _sum;
  int _count, _write_idx;

 public:
  explicit Average(int points) : _points(points), _data(nullptr) {
  }
  ~Average() {
    if (_data != nullptr)
      delete[] _data;
  }
  void begin() {
    _data = new float[_points];
    clear();
  }
  void begin(float v) {
    begin();
    add(v);
  }
  void clear() {
    _count = 0;
    _write_idx = 0;
    _sum = 0.f;
  }
  float add(float v) {
    if (_data == nullptr) return 0.f;
    if (_count < _points) {
      ++_count;
    } else {
      _sum -= _data[_write_idx];
    }
    _sum += v;
    _data[_write_idx] = v;
    _write_idx = (_write_idx + 1) % _points;
    return get();
  }
  float get() const {
    return _count > 0.f ? _sum / _count : 0.f;
  }
};

#endif  // AVERAGE_H_
