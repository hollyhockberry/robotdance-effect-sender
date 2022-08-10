// Copyright (c) 2022 Inaba (@hollyhockberry)
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include <M5StickC.h>
#include <esp_now.h>
#include <WiFi.h>
#include "average.h"

namespace {

bool _moving = false;

// 動作継続タイマ
unsigned long _timer;

// 再送タイマ
unsigned long _next_sendtimer;

// 現在の加速度
float _x, _y, _z;
constexpr int POINTS = 5;
Average _ax(POINTS), _ay(POINTS), _az(POINTS);

constexpr uint8_t CHANNEL = 1;
constexpr uint8_t BROADCAST[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void setup_esp_now() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (::esp_now_init() != ESP_OK) {
    ::esp_restart();
  }
  esp_now_peer_info_t info = {};
  ::memcpy(info.peer_addr, BROADCAST, 6);
  info.channel = CHANNEL;
  info.encrypt = false;
  if (::esp_now_add_peer(&info) != ESP_OK) {
    ::esp_restart();
  }
}

void send_esp_now(bool force) {
  const auto now = ::millis();
  if (force || _next_sendtimer < now) {
    uint8_t i = _moving ? 1 : 0;
    ::esp_now_send(BROADCAST, &i, 1);
    digitalWrite(G10, !_moving ? HIGH : LOW);
    _next_sendtimer = now + 200;
  }
}

}  // namespace

void setup() {
  M5.begin();
  ::pinMode(G10, OUTPUT);

  M5.Imu.Init();
  M5.IMU.getAccelData(&_x, &_y, &_z);
  _ax.begin(_x);
  _ay.begin(_y);
  _az.begin(_z);

  setup_esp_now();
}

void loop() {
  float cx, cy, cz;
  M5.IMU.getAccelData(&cx, &cy, &cz);
  cx = _ax.add(cx);
  cy = _ay.add(cy);
  cz = _az.add(cz);

  const float dx = ::abs(cx - _x);
  const float dy = ::abs(cy - _y);
  const float dz = ::abs(cz - _z);

  const bool moving = _moving;
  const auto now = ::millis();
  constexpr float thres = 0.04f;
  if ((dx > thres) || (dy > thres) || (dz > thres)) {
    _moving = true;
    _timer = now + 150;
  }
  if (_moving && _timer < now) {
    _moving = false;
  }
  send_esp_now(moving != _moving);

  _x = cx;
  _y = cy;
  _z = cz;
  ::delay(50);
}
